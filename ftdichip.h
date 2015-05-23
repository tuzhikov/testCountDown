#ifndef FTDICHIP_H
#define FTDICHIP_H

#include <QObject>
#include <windows.h>
#include <QQueue>
#include <QTimer>
#include <QProgressDialog>
#include"QColor"
#include "FTDI\FTD2XX.H"

namespace ANWR_PROTOCOL{
    enum RETURN_ANSWER{retOK=0x0001,retError=0x0002,retGet=0x0004,
                       retNoAnsError=0x0040,retIncorData=0x0200};
}
class ParameterUSB{

    quint64 TimeDelay;
    quint64 Speed;
    quint8  Databit;
    quint8  StopBit;
    quint8  Parity;

public:
    ParameterUSB(quint64  TimeDelay=150,quint64 Speed=FT_BAUD_19200,quint8 Databit=FT_BITS_8,
                 quint8 StopBit=FT_STOP_BITS_1,quint8 Parity=FT_PARITY_NONE):
        TimeDelay(TimeDelay),Speed(Speed),Databit(Databit),StopBit(StopBit),Parity(Parity){
            }
    void setTimeDelay(const quint64 timeDelay){TimeDelay=timeDelay;}
    void setSpeed(const quint64 speed)   {Speed=speed;}
    void setDataBit(const quint8 databit){Databit=databit;}
    void setStopBit(const quint8 stopbit){StopBit=stopbit;}
    void setParity(const quint8 parity)  {Parity=parity;}

    quint64 retTimeDelay(void) {return TimeDelay;}
    quint64 retSpeed(void) {return Speed;}
    quint8 retDataBit(void){return Databit;}
    quint8 retStopBit(void){return StopBit;}
    quint8 retParity(void) {return Parity;}


};

class ftdiChip : public QObject
{
    Q_OBJECT
    enum State{stOpen,stCounter,stWrite,stRead,stClose};
    State stat;
    static const int LengPckg = 10;
    static const char ADDR_REMOTE = 0x0B;
    static const int bCollin = 5;
    static const int bLine = 64;
    const char *pMask;
    char *BufPtrs[bCollin];
    char Buffer1[bLine];
    char Buffer2[bLine];
    char Buffer3[bLine];
    char Buffer4[bLine];
    bool ModeReceiver;
    ParameterUSB parameter; // link parameter
    QQueue<QByteArray> listCMD;
    QTimer *timerRead;

    /* local function */
    void Close();
    quint16 Open();
    quint16 Read(QByteArray &buff);
    quint16 Write(void *pdata,DWORD leng,DWORD *nMin);
    quint16 Bit8Write(QByteArray &buff, quint16 repet);
    quint16 Bit9Write(QByteArray &buff, quint16 repet);
    quint16 setParametersUSB(quint64 Speed=FT_BAUD_19200,
                             quint8 DataBit=FT_BITS_8,
                             quint8 StopBit=FT_STOP_BITS_1,
                             quint8 Parity=FT_PARITY_NONE);
    ANWR_PROTOCOL::RETURN_ANSWER CheckingReceivedPacket(const QByteArray&);
protected:
    FT_HANDLE hdUSB;
    DWORD numDevice;
public:
    enum retFunct{retOk=0x0001,retErr=0x0002,retBusyDevice=0x0004,retChoice=0x0008};
    explicit ftdiChip(QObject *parent = 0);
    virtual void sendMessage(QQueue<QByteArray> &ba);
    virtual void sendMessage(QByteArray &ba);
    virtual void stopMessage();
    quint16 SearchDevice();
    bool setParameterDevice(const QStringList&);
signals:
    void signalStart();
    void signalStop();
    void signalStep();
    void signalEnd(const bool);
    void signalSearchUsb(QStringList&,quint16,quint16);
    void signalSendMessage(const bool,const QByteArray&,const QColor&); // send protocol
    void signalStatusError(const QString&,const bool);
    void signalStatusOk(const QString&);
    void signalMessageOk(const QString&);
    void signalMessageError(const QString&);
    void signalProgressRange(const int,const int);
    void signalProgressValue(const int,const bool);
private slots:
    void on_Machine();
public slots:
    quint16 on_EEPROM();
    quint16 on_SearchDevice();
    void on_SetNumberUSB(const quint64 number);
    void on_ParameterLink(const quint64 td,
                          const quint64 sp,
                          const quint8 db,
                          const quint8 sb,
                          const quint8 pr);
};

#endif // FTDICHIP_H
