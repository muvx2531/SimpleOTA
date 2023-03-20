#ifndef SIMPLEOTA_H
#define SIMPLEOTA_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QFileDialog>

namespace Ui {
class SimpleOTA;
}

typedef struct
{
    qint64 Lineincrement;
    qint64 LineTotal;
    qint64 LinerequestedStart;
    qint64 LinerequestedStop;
}HEXinfo;

class SimpleOTA : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimpleOTA(QWidget *parent = nullptr);
    ~SimpleOTA();

    void checksum();


        QTcpSocket  *OTAsocket;
        QFile       File;
        QString      BootloaderInstruction;
        HEXinfo OTAfile;
public slots:
        void onReadyRead();
        void onDisconnected();
        void onConnected();

private slots:
    void on_Connect_clicked();

    void on_Disconnect_clicked();

    void on_StartProgram_clicked();

    void on_RUN_APP_clicked();

    void on_actionOpen_triggered();

    void on_ButtonClearAPP_clicked();

private:
    Ui::SimpleOTA *ui;

};

#endif // SIMPLEOTA_H
