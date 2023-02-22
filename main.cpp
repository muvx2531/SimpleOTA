#include "simpleota.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimpleOTA w;
    w.show();

    return a.exec();
}
