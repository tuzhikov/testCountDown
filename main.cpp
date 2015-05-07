#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(QCurrentResource);
    QTranslator trn;
    trn.load(QString(":/TR/TR/TestCountDown_en"));
    a.installTranslator(&trn);
    MainWindow w;
    w.move((QApplication::desktop()->width()-w.width())/2,
                   (QApplication::desktop()->height()-w.height())/2);
    w.show();
    w.setWindowIcon(QPixmap(":/ICO/ICO/elintel.ico"));
    w.setWindowTitle(QObject::tr("testCountDown"));

    return a.exec();
}
