#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QUdpSocket>
#include <QMessageBox>
#include <memory>
#include <common.h>

int totalrcv = 0;  //колличество принятых данных

std::shared_ptr<QUdpSocket> sock_rcv; //указатель на QUdpsocket

//функция добавления доступных ip адаптеров
QMap<QString, QList<QHostAddress>> ifaces() {
    //регулярное выражения получения ip из текста
    QRegularExpression ipRegExp(R"(^(?<ip>((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)))$)");

    QMap<QString, QList<QHostAddress>> interfaces;
    //цикл прохода по доступным сетевым интерфейсам
    for(auto item : QNetworkInterface::allInterfaces()) {
        if(item.isValid() && item.flags().testFlag(QNetworkInterface::IsUp))
        {
            QList<QHostAddress> hosts;
            //доступные адреса адаптеров
            auto adresses = item.addressEntries();

            for(auto a : adresses) {
                //получаем ip  из регулярного выражения
                auto ip = ipRegExp.match(a.ip().toString());
                if(ip.hasMatch())
                {
                    hosts << a.ip();
                }
            }

            //добавляем ip в мапу
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
    QStringList strlist;

    ui->adaptersend->addItem("not bound");  //-------+
    ui->adapterrcv->addItem("not bound");   //добавляем элемент в комбобокс есди не определен адаптер(для широковещяния)
    ui->transformcb->addItems(QStringList()<<"HEX"<<"BIN"<<"NOT TRANSFORM");
    //получаем Map доступных адаптеров
    auto list = ifaces();

    //проходим по всей Map и пердаём в список
    for(auto hostlist : list) {
        //проходим по списку и получаем доступные ip адаптеров
        for(auto host : hostlist){
            ui->adaptersend->addItem(host.toString()); // Добавляем IP адаптеров в комбобокс
            ui->adapterrcv->addItem(host.toString());  //
        }
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}


//при нажатии кнопки "Отправить"
void MainWindow::on_sendbt_clicked()
{
    //регулярное выражения для получения порта
    QRegularExpression rePort(R"(^(?<port>[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-3][0-9]|6553[0-5])$)");
    //матчим порт из Textline
    auto matchPort = rePort.match(ui->portsend->text());
    //если ввели не правильное значение порта
    if(!matchPort.hasMatch()) {
        QMessageBox::warning(this, "Warning", "Invalid port");
        return;
    }
    //присваиваем заматченый порт в int
    uint16_t port = matchPort.captured("port").toUShort();

    //Определяем QhostAddress широковещательным
    QHostAddress dest = QHostAddress::Broadcast;

    //если ввели IP
    if(!ui->ipv4send->text().isEmpty()){

        //регулярное выражения для получения ip из текста
        QRegularExpression ipRegExp(R"(^(?<ip>((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)))$)");

        //Матчим ip  из textimput
        auto matchIp = ipRegExp.match(ui->ipv4send->text());
        //если ввели не корректный ip
        if(!matchIp.hasMatch()){
            QMessageBox::warning(this, "Warning", "Invalid IP address");
            return;
        }
        //присваиваем заматченый IP  для адреса куда отправляем
        dest = QHostAddress(matchIp.captured("ip"));
    }

    QUdpSocket sock;
    QHostAddress adapter;
    //если выбран в комбобоксе "not bound"  присваеваем адаптеру любой IPv4
    if(ui->adaptersend->currentText() == "not bound"){
        adapter = QHostAddress::AnyIPv4;
    }
    //если выбранн любой другой доступный IP присваиваем его адаптеру
    else adapter = QHostAddress(ui->adaptersend->currentText());
    //биндем сокет
    //   |ip    |порт    |ENUM  передаем в аргументы, чтоб остальные функции могли пользоваться сокетом
    sock.bind(adapter, port, QAbstractSocket::ShareAddress);
    //если не ввели ничего в строку "Данные"
    if(ui->textinput->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Data is empty");
        return;
    }
    //введеные данные переводим в 8 бит и массив байт
    QByteArray data = ui->textinput->text().toLocal8Bit();

    //отправляем данные через сокет
    //            массив байт
    //                  | адрес куда отправить
    //                  |      |    порт
    if(!sock.writeDatagram(data, dest, port)) {
        //если отправка не получилась
        QMessageBox::warning(this, "Warning", "Error send");

    }

}

//кнопка слушать поключение
void MainWindow::on_connectbt_clicked()
{
    //если есть UdpSocket мы его удаляем
    if(sock_rcv) {
        sock_rcv = nullptr;
    }
    else {
        // если нет сокет открываем его,                        | лямбда функция, для удаления указателя, при удалении меняем название кнопки на "Listen"
        sock_rcv = std::shared_ptr<QUdpSocket>(new QUdpSocket, [&](QUdpSocket *p){
                ui->connectbt->setText("Listen");
                ui->from->setText("From: ");
                ui->labelconn->clear();

                delete p;
    });
        QRegularExpression rePort(R"(^(?<port>[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-3][0-9]|6553[0-5])$)");
        auto matchPort = rePort.match(ui->portsend->text());
        //если некорректное значение порта
        if(!matchPort.hasMatch()) {

            QMessageBox::warning(this, "Warning", "Invalid port");
            //обнуляем указатель сокет ресирвер
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

        //биндим сокет ресирверу адаптер(доступный ip адрес) и порт,
        sock_rcv->bind(adapter, port, QAbstractSocket::ShareAddress);

        //при подключении меняем имя кнопки на "Close"
        ui->connectbt->setText("Close");

        ui->labelconn->setText(adapter.toString() + ":" + QString::number(port));



        //слушаем UDP     Готовность чтения данных             Лямбда фунция для принятия данных из сокета
        connect(sock_rcv.get(), &QUdpSocket::readyRead, this, [&](){
            //если есть датаграммы возвращает true
            while (sock_rcv->hasPendingDatagrams()) {
                QByteArray data;
                QHostAddress from;
                data.resize(sock_rcv->pendingDatagramSize()); // принимаем размер датаграммы из сокета

                sock_rcv->readDatagram(data.data(), data.size(), &from); //запись в массив данных полученых из сокета, и принимаем размер полученных данных

                ui->lenrcv->setNum(data.size()); //получаем размер байт преданных данных (в нашем случаем char)
                ui->totalrcv->setNum(++totalrcv);  // колличество  раз принятых данных

                ui->from->setText("From: " + from.toString());
                QString transformbox =  ui->transformcb->currentText();
                QString result;
                if(transformbox == "BIN") {
                    result = glb::bin(data);
                }
                else if (transformbox == "HEX") {
                    result = glb::hex(data);
                }
                else
                    result = data;


                ui->textBrowser->setText(result); //  выводим полученные данные в HEX


            }
        });
    }
}

void MainWindow::on_clearbt_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_actionAbout_QT_triggered()
{
    QMessageBox::aboutQt(this);
}
