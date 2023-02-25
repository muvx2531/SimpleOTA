#include "simpleota.h"
#include "ui_simpleota.h"
#include "QDebug"
#include "QMessageBox"
#include <QChar>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>


SimpleOTA::SimpleOTA(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SimpleOTA)
{
    ui->setupUi(this);
    OTAsocket = nullptr;
    OTAfile.LineTotal = 0;
    OTAfile.Lineincrement = 0;


//    /**************Test Json oject*****************/
//    QByteArray jsonData("{\"name\":\"John\", \"age\":30, \"car\":null}");
//    if(jsonData.isEmpty() == true) qDebug() << "Need to fill JSON data";

//    //Assign the json text to a JSON object
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
//    if(jsonDocument.isObject() == false) qDebug() << "It is not a JSON object";

//    QJsonObject object = jsonDocument.object();

//    QString name;
//    name.append(object.value("name").toString());

//    qDebug()<<name<<" "<<object.value("age").toInt();
//    /************ END Test Json oject ***************/
}

SimpleOTA::~SimpleOTA()
{
    delete ui;
    if(OTAsocket != nullptr)delete  OTAsocket;
}

void SimpleOTA::on_Connect_clicked()
{
    if(ui->Interface_TCP)
    {
        if(OTAsocket == nullptr)
        {
            OTAsocket =new QTcpSocket(this);
            bool pb;
            OTAsocket->connectToHost(ui->IP->text(),static_cast<quint16>(ui->TCP_Port->text().toInt(&pb)));
            connect(OTAsocket, SIGNAL(connected()), this, SLOT(onConnected()));
            connect(OTAsocket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
            connect(OTAsocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
            ui->Connect->setEnabled(0);
            ui->IP->setEnabled(0);
            ui->TCP_Port->setEnabled(0);
        }
    }
    else if(ui->Interface_Serial)
    {

    }
}

void SimpleOTA::onReadyRead()
{
    QByteArray tmp(OTAsocket->readAll());
    qDebug()<<"TCP recieved data >> "<<tmp<<"size  in =="<<tmp.size();

    //Assign the json text to a JSON object
    QJsonDocument jsonDocument = QJsonDocument::fromJson(tmp);
    if(jsonDocument.isObject() == false) qDebug() << "It is not a JSON object";
    else{
        QJsonObject object = jsonDocument.object();
        QString CRC;

        CRC.append(object.value("CRC").toString());
        //if(CRC == TRUE)
        {
            OTAfile.LinerequestedStart = static_cast<qint64>(object.value("startLine").toInt());
            OTAfile.LinerequestedStop = static_cast<qint64>(object.value("stopLine").toInt());
        }
    }
    qDebug()<<"Line start = "<<OTAfile.LinerequestedStart<<"Line stop = "<<OTAfile.LinerequestedStop;
    if((OTAsocket != nullptr) && (BootloaderInstruction.size() !=0)){

        QByteArray package;
        qint64 linenumber=0;
        for(quint64 i= 0;i<static_cast<quint64>(BootloaderInstruction.size());i++)
        {

            if(BootloaderInstruction.at(static_cast<int>(i)) == ':')++linenumber;
            if((linenumber >= OTAfile.LinerequestedStart) && (linenumber <= OTAfile.LinerequestedStop))
            {
                package.append(BootloaderInstruction.at(static_cast<int>(i)));
            }
            //qDebug()<<linenumber<<"  "<<OTAfile.LinerequestedStart<<" "<<OTAfile.LinerequestedStop;
        }

       OTAsocket->write(package);

       QString print;
       print.append(QString::number(OTAfile.LineTotal));
       print.append("   Line request = ");
       print.append(QString::number(OTAfile.LinerequestedStart));
       print.append(" to ");
       print.append(QString::number(OTAfile.LinerequestedStop));
       ui->Message->append(print);
       ui->Message->moveCursor(QTextCursor::End);
       qDebug()<<package;
    }


    //Handle package requested
}


void SimpleOTA::onConnected()
{
    qDebug()<<"TCP connected";
    ui->Message->setText("+++Connected+++");
    if(BootloaderInstruction.size() != 0)
    {
        QString Total(QString::number(OTAfile.LineTotal));

          QString sum(".Hex ready Total Line = ");
          sum.append(Total);
        ui->Message->append(sum);
    }
    else
    {
        ui->Message->append("\r\n!!! Please insert HEX !!!");
    }
    //Interval->start(1000);
}

void SimpleOTA::onDisconnected()
{
    qDebug()<<"TCP DIS connected";
}

void SimpleOTA::on_Disconnect_clicked()
{
    if(OTAsocket != nullptr)delete  OTAsocket;
    OTAsocket = nullptr;
    ui->Connect->setEnabled(1);
    ui->IP->setEnabled(1);
    ui->TCP_Port->setEnabled(1);
}

void SimpleOTA::on_StartProgram_clicked()
{
    char sumxor=0;
    QByteArray Start("HEX=");
    Start.append(QString::number(OTAfile.LineTotal));
    Start.append("*");

    for(quint16 i=0;i<Start.size();i++)
    {
        sumxor = static_cast<quint8>(Start.at(i))^sumxor;
    }
    char bLow,bHigh;
    bLow = sumxor&0x0F;
    bHigh= sumxor>>4;
    if(bHigh > 9)bHigh = bHigh+0x37;
    else bHigh = bHigh+0x30;
    if(bLow > 9)bLow = bLow+0x37;
    else bLow = bLow+0x30;
    Start.append(bHigh);
    Start.append(bLow);

    OTAsocket->write(Start);
    qDebug()<<Start<<" "<<sumxor;
}

void SimpleOTA::on_RUN_APP_clicked()
{
    QByteArray Start("RUN APPr\r\n");
    OTAsocket->write(Start);
}

void SimpleOTA::on_actionOpen_triggered()
{
    qDebug()<<"Open file";
    QString fileName = QFileDialog::getOpenFileName(this,"Open File","","*.hex");

    File.setFileName(fileName);
    qDebug()<<"file name is "<<fileName;
    if(!File.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug()<< " Could not open file for writing";
        return;
    }
    else
    {
        QTextStream in(&File);
        BootloaderInstruction.clear();
        ui->Message->clear();
        BootloaderInstruction.append(in.readAll());
        OTAfile.LineTotal = 0;
        OTAfile.Lineincrement = 0;
        for(quint64 i= 0;i<static_cast<quint64>(BootloaderInstruction.size());i++)
        {
            if(BootloaderInstruction.at(static_cast<int>(i)) == '\n')OTAfile.LineTotal++;
        }
        qDebug()<<"total line ="<<OTAfile.LineTotal;

//       ui->Message->append(BootloaderInstruction);
//        ui->Hexcode->clear();
//        ui->Hexcode->append(BootloaderInstruction);
//        qDebug()<<"Data = "<<BootloaderInstruction.at(0)<<BootloaderInstruction.at(1)<<BootloaderInstruction.at(2);
        File.close();


//        if(OTAsocket != nullptr)
//        {
//            if(OTAsocket->state() == QAbstractSocket::ConnectedState )
//            {
                ui->Message->setText("+++.Hex ready+++");
                QString Total(QString::number(OTAfile.LineTotal));

                  QString sum(".Hex ready Total Line = ");
                  sum.append(Total);
                ui->Message->append(sum);
//            }
//        }
    }
}

void SimpleOTA::checksum()
{

}
