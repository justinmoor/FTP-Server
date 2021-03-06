#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    server = new Server(this);
    overview = new UsersOverview;
    ui->connectedUsersTable->setContextMenuPolicy(Qt::CustomContextMenu);
    setStandardSettings();

    ui->logTab->removeTab(1);
    ui->usersTab->removeTab(1);

    connect(server, SIGNAL(sendToLogServer(QString)), this, SLOT(appendLogServer(QString)));
    connect(server, SIGNAL(sendToLogClient(QString,QString)), SLOT(appendLogClient(QString, QString)));
    connect(server, SIGNAL(sendToTable(QString, QString, QString)), this, SLOT(appendTable(QString, QString, QString)));
    connect(server, SIGNAL(removeFromTableSignal(int)), this, SLOT(removeFromTable(int)));
    connect(this, SIGNAL(deleteUser(int)), server, SLOT(deleteUserServer(int)));
    connect(this, SIGNAL(deleteUser(int)), server, SLOT(deleteUserServer(int)));
    connect(this, SIGNAL(getUsers()), overview, SLOT(getUsers()));
    connect(ui->connectedUsersTable, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(deleteUserMenu(const QPoint &)));

    ui->actionClose_connection->setEnabled(false);
    ui->connectedUsersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete server;
}

void MainWindow::on_actionOpen_connection_triggered()
{
    ui->actionOpen_connection->setEnabled(false);
    ui->actionClose_connection->setEnabled(true);
    appendLogServer("Starting server!");

    server->startServer();
}

void MainWindow::on_actionClose_connection_triggered()
{
    ui->actionClose_connection->setEnabled(false);
    ui->actionOpen_connection->setEnabled(true);

    appendLogServer("Shutting down...");

    server->stopServer();
    ui->connectedUsersTable->setRowCount(0);
}

void MainWindow::appendLogServer(QString msg){
    QDateTime current = QDateTime::currentDateTime();
    ui->log->appendPlainText(current.toString() + " - [SERVER] > " + msg);
}

void MainWindow::appendLogClient(QString username, QString message){
    QDateTime current = QDateTime::currentDateTime();
    ui->log->appendPlainText(current.toString() + " - ["+username+"] > " + message.trimmed());
}

void MainWindow::appendTable(QString id, QString ip, QString username){
    ui->connectedUsersTable->insertRow(ui->connectedUsersTable->rowCount());
    ui->connectedUsersTable->setItem(ui->connectedUsersTable->rowCount()-1, 0, new QTableWidgetItem(id));
    ui->connectedUsersTable->setItem(ui->connectedUsersTable->rowCount()-1, 1, new QTableWidgetItem(username));
    ui->connectedUsersTable->setItem(ui->connectedUsersTable->rowCount()-1, 2, new QTableWidgetItem(ip));
}

void MainWindow::deleteUserMenu(const QPoint &click){
    QMenu menu(this);
    QAction *u = menu.addAction(tr("Kick user"));
    QAction *a = menu.exec(ui->connectedUsersTable->viewport()->mapToGlobal(click));

    if(a == u){
        int selectedRow = ui->connectedUsersTable->selectionModel()->currentIndex().row();
        int selectedId = ui->connectedUsersTable->item(selectedRow, 0)->text().toInt();
        emit deleteUser(selectedId);
        ui->connectedUsersTable->removeRow(selectedRow);
    }
}

void MainWindow::removeFromTable(int id){
    for(int i = 0; i < ui->connectedUsersTable->rowCount(); i++){
        if(ui->connectedUsersTable->item(i, 0)->text().toInt() == id){
            ui->connectedUsersTable->removeRow(i);
        }
    }
}

void MainWindow::on_actionClear_current_log_triggered()
{
    ui->log->clear();
}

void MainWindow::on_actionAdd_user_triggered()
{
    addUser userAdd;
    userAdd.setModal(true);
    userAdd.exec();
}

void MainWindow::on_actionUser_list_triggered()
{
    emit getUsers();
    overview->show();
}

void MainWindow::on_actionConfigure_server_triggered()
{
    Settings settings;
    settings.setModal(true);
    settings.exec();
}

void MainWindow::setStandardSettings(){
    QSettings settings;
    settings.setValue("serverPort", (settings.value("serverPort").isNull()) ? 1234 : settings.value("serverPort").toInt());
    settings.setValue("maxUsers", (settings.value("maxUsers").isNull()) ? 5 : settings.value("maxUsers").toInt());
    settings.setValue("rootPath", (settings.value("rootPath").isNull()) ? QDir::homePath() : settings.value("rootPath").toString());
    settings.setValue("welcomeMessage", (settings.value("welcomeMessage").isNull()) ? "Welcome to Jussie's FTP Server!" : settings.value("welcomeMessage").toString());
    settings.setValue("allowAnonUsers", (settings.value("allowAnonUsers").isNull()) ? false : settings.value("allowAnonUsers").toBool());
}
