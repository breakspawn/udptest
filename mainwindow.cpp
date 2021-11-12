#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QUdpSocket>
#include <QMessageBox>
#include <memory>
#include <common.h>

int totalrcv = 0;

std::shared_ptr<QUdpSocket> sock_rcv;

QMap<QString, QList<QHostAddress>> ifaces() {
    QRegularExpression ipRegExp(R"(^(?<ip>((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)))$)");

    QMap<QString, QList<QHostAddress>> interfaces;
    for(auto item : QNetworkInterface::allInterfaces()) {
        if(item.isValid() && item.flags().testFlag(QNetworkInterface::IsUp))
        {
            QList<QHostAddress> hosts;
            auto adresses = item.addressEntries();
            for(auto a : adresses) {
                auto ip = ipRegExp.match(a.ip().toString());
                if(ip.hasMatch())
                {
                    hosts << a.ip();
                }
            }

            interfaces.insert(item.name(), hosts);
        }
    }
    return interfaces;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->adaptersend->addItem("not bound");
    ui->adapterrcv->addItem("not bound");
    auto list = ifaces();

    for(auto hostlist : list) {
        for(auto host : hostlist){
            ui->adaptersend->addItem(host.toString());
            ui->adapterrcv->addItem(host.toString());
        }
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_sendbt_clicked()
{
    QRegularExpression rePort(R"(^(?<port>[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-3][0-9]|6553[0-5])$)");
    auto matchPort = rePort.match(ui->portsend->text());
    if(!matchPort.hasMatch()) {
        QMessageBox::warning(this, "Warning", "Invalid port");
        return;
    }
    uint16_t port = matchPort.captured("port").toUShort();
    QHostAddress dest = QHostAddress::Broadcast;
    if(!ui->ipv4send->text().isEmpty()){
        QRegularExpression ipRegExp(R"(^(?<ip>((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)))$)");
        auto matchIp = ipRegExp.match(ui->ipv4send->text());
        if(!matchIp.hasMatch()){
            QMessageBox::warning(this, "Warning", "Invalid IP address");
            return;
        }
        dest = QHostAddress(matchIp.captured("ip"));
    }
    QUdpSocket sock;
    QHostAddress adapter;
    if(ui->adaptersend->currentText() == "not bound"){
        adapter = QHostAddress::AnyIPv4;
    }
    else adapter = QHostAddress(ui->adaptersend->currentText());
    sock.bind(adapter, port, QAbstractSocket::ShareAddress);
    if(ui->textinput->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Data is empty");
        return;
    }
    QByteArray data = ui->textinput->text().toLocal8Bit();
    if(!sock.writeDatagram(data, dest, port)) {
        QMessageBox::warning(this, "Warning", "Error send");

    }

}

void MainWindow::on_connectbt_clicked()
{
    if(sock_rcv) {
        sock_rcv = nullptr;
    }
    else {
        sock_rcv = std::shared_ptr<QUdpSocket>(new QUdpSocket, [&](QUdpSocket *p){
                ui->connectbt->setText("Listen");
                delete p;
    });
        QRegularExpression rePort(R"(^(?<port>[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-3][0-9]|6553[0-5])$)");
        auto matchPort = rePort.match(ui->portsend->text());
        if(!matchPort.hasMatch()) {
            QMessageBox::warning(this, "Warning", "Invalid port");
            sock_rcv = nullptr;
            return;
        }
        uint16_t port = matchPort.captured("port").toUShort();
        QHostAddress adapter;

        if(ui->adapterrcv->currentText() == "not bound"){
            adapter = QHostAddress::AnyIPv4;
        }
        else
            adapter = QHostAddress(ui->adapterrcv->currentText());

        sock_rcv->bind(adapter, port, QAbstractSocket::ShareAddress);

        ui->connectbt->setText("Close");

        connect(sock_rcv.get(), &QUdpSocket::readyRead, this, [&](){
            while (sock_rcv->hasPendingDatagrams()) {
                QByteArray data;
                data.resize(sock_rcv->pendingDatagramSize()); // принимаем размер датаграммы из сокета
                sock_rcv->readDatagram(data.data(), data.size());

                ui->lenrcv->setNum(data.size());
                ui->textBrowser->setText(glb::hex(data));
                ui->totalrcv->setNum(++totalrcv);



            }
        });
    }
}

void MainWindow::on_clearbt_clicked()
{
    ui->textBrowser->clear();
}
