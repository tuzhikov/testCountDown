#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QInputDialog>
#include <QProgressBar>
#include "ftdichip.h"
#include "parserxml.h"
#include "dialog.h"

namespace Ui {
class MainWindow;
}
/*Checksum calculation*/
class CRC
{
public:
    //CRC8
    static quint8 Bit8(QByteArray &data){
      quint8 CRC8=0;
      foreach(char byte,data)
        CRC8 += byte;
      return CRC8;
      }
    //CRC-16
    static quint16 Bit16(const QByteArray &data){
        quint16 CRC16 = 0xFFFF;
        quint16 leng = data.size();
        const quint8 *pMess = (quint8*)(data.data());

        while(leng--)
            {
            CRC16^=*pMess;
            for(int i=0;i<8;i++)
                {
                if(CRC16&0x0001){ // low bit
                    CRC16>>=1;
                    CRC16^=0xA001;
                    }
                else{
                    CRC16>>=1;
                    }
                }
            pMess++;
            }
        return CRC16;
        }
};
class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum STATELIGHT{stRed,stActionRed,stGreen,stActionGreen,stEndProgress};

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static QString ShiftCRC8(const QByteArray &);
    static QString ShiftCRC(const QByteArray &);
private:
    Ui::MainWindow *ui;
    QTimer *timerCount;
    ftdiChip *usb;
    QProgressBar *progress_status;
    QStringList strParamXML;
    bool ProgressEnd;

    void CreateStyle();
    void writeSettings();
    void readSettings();
    void SegmentStyleLCD(QColor color);
    bool showCountDownBiard(QTime &);
    void ShowParametersForm(const QStringList &);
    void EnablePanel(const bool enabled = false);
    QString StringHEX(const QString str, const bool visible = true)const;
    QString ConvertTimeToSec(const QTime)const;
    QString CollectDataToWidget(const int,const int)const;
    QString CollectDataToWidgetPageAuto(const int) const;
    QString CollectDataToWidgetPageTest() const;
    QString CollectDataToWidgetPageService() const;
    QString (MainWindow::*CollectDataToWidgetPage[3])();
    void CollectDataToSen(QString &str,const int,const bool split = false);
private slots:
    void on_About(void);
    void on_StartButton();
    void on_StopButton();
    void on_Progress();
    void on_Manual();
    void on_SendUsbDebug(QString);
    void on_GetMessageOutTextEdit(const bool direction,const QByteArray &st, const QColor &color);
    void on_SetMessageStatusBarError(const QString &, const bool fShowMin = false);
    void on_SetMessageStatusOk(const QString &message);
    void on_USB_choice(QStringList&,quint16,quint16);
    void on_setValueProgress(const int,const bool);
    void on_setValueBright();
signals:
    void slSetValueAdr(int);
    void slNumberUSB(quint64);
    void slPrmUSB(const quint64,const quint64,const quint8,const quint8,const quint8);
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
