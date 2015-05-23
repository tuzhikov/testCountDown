#include "ftdichip.h"
#include <QMessageBox>
#include <QStringList>
#include "parserxml.h"

ftdiChip::ftdiChip(QObject *parent) :
  QObject(parent),stat(stOpen),timerRead(new QTimer(parent))
{
BufPtrs[0] = Buffer1;
BufPtrs[1] = Buffer2;
BufPtrs[2] = Buffer3;
BufPtrs[3] = Buffer4;
BufPtrs[4] = NULL;
connect(this,SIGNAL(signalStart()),this,SIGNAL(signalStep()),Qt::DirectConnection);
connect(this,SIGNAL(signalStep()),this,SLOT(on_Machine()),Qt::DirectConnection);
connect(timerRead,SIGNAL(timeout()),this,SLOT(on_Machine()),Qt::DirectConnection);
}
/*Write paskage to TOO*/
void ftdiChip::on_Machine()
{
static QQueue<QByteArray> templistCMD;
static quint16 retAnswer;
static int CurrentValue,ValueMax;
switch(stat)
    {
    case stOpen:
        if(!listCMD.isEmpty()){
            if(Open()==retOk){
                stat=stCounter;
                disconnect(this,SIGNAL(signalStart()),this,SIGNAL(signalStep()));
                setParametersUSB(parameter.retSpeed(),parameter.retDataBit(),parameter.retStopBit(),parameter.retParity());
                timerRead->setInterval(parameter.retTimeDelay());
                templistCMD = listCMD;
                CurrentValue = 0;
                ValueMax = 3*templistCMD.count();
                emit signalProgressRange(0,ValueMax);
                emit signalStep();
                }else {
                emit signalMessageError(tr("<CENTER><b>Not found KIT USN_RS485!</CENTER></b>"));
                emit signalStop();
                }
            }
        listCMD.clear();
        return;
    case stCounter:
        if(!templistCMD.isEmpty()){
            emit signalProgressValue(++CurrentValue,true);
            stat=stWrite;
            }else{
            stat=stClose;
            }
        emit signalStep();
        return;
    case stWrite:
        {
        QByteArray cmd(templistCMD.dequeue());//long Time;
        quint16 result = Bit8Write(cmd,1);
        emit signalSendMessage(false,cmd,Qt::green);
        if(result!=retOk){
            templistCMD.clear();stat=stCounter;emit signalStep();}
        stat=stRead;
        timerRead->start();
        emit signalProgressValue(++CurrentValue,true);
        }
        return;
    case stRead:
        {
        QByteArray resBuff;
        quint16 retRead =Read(resBuff);
        emit signalSendMessage(true,resBuff,Qt::darkBlue);
        retAnswer = CheckingReceivedPacket(resBuff);
        if(retAnswer&ANWR_PROTOCOL::retGet){
          return; // next package
          }
        if((retAnswer&(~(ANWR_PROTOCOL::retOK)))
             &&
           (retRead&(ftdiChip::retErr|ftdiChip::retBusyDevice))){// stop sending
          templistCMD.clear();
          }
        stat=stCounter;
        timerRead->stop();
        emit signalProgressValue(++CurrentValue,true);
        emit signalStep();
        }
        return;
    case stClose:
        Close();
        connect(this,SIGNAL(signalStart()),this,SIGNAL(signalStep()),Qt::DirectConnection);
    default:
        emit signalEnd(true);
        stat = stOpen;
        timerRead->stop();
        emit signalProgressValue(0,false);
        if(retAnswer&(ANWR_PROTOCOL::retOK)) // it is not error
          emit signalStatusOk(tr("Data is written successfully!"));
        if(retAnswer&ANWR_PROTOCOL::retError) // it is error
          emit signalStatusError(tr("Error response!"),false);
        if(retAnswer&ANWR_PROTOCOL::retNoAnsError) // it is error
          emit signalStatusError(tr("Device does not answer!"),false);
        if(retAnswer&ANWR_PROTOCOL::retIncorData)
          emit signalStatusError(tr("Data is incorrect!"),false);
        emit signalStep();
        return;        
  }
}
/*Set parametr Link*/
void ftdiChip::on_ParameterLink(const quint64 td,
                                const quint64 sp,
                                const quint8 db,
                                const quint8 sb,
                                const quint8 pr)
{
parameter.setTimeDelay(td);
parameter.setSpeed(sp);
parameter.setDataBit(db);
parameter.setStopBit(sb);
parameter.setParity(pr);
}
/*send group message*/
void ftdiChip::sendMessage(QQueue<QByteArray> &ba)
{
listCMD.clear();
listCMD = ba;
emit signalStart();
}
/* send one message*/
void ftdiChip::sendMessage(QByteArray &ba)
{
listCMD.enqueue(ba);
emit signalStart();
}
/*Stop send message*/
void ftdiChip::stopMessage()
{
listCMD.clear();
connect(this,SIGNAL(signalStart()),this,SIGNAL(signalStep()),Qt::DirectConnection);
}
/*Search device parametr*/
quint16 ftdiChip::on_SearchDevice()
{
FT_STATUS ftStatus;
DWORD numDev; // колличество USB устройств
BYTE n=(BYTE)NULL;
QStringList itm;
quint16 Return;
quint8 counDevice = (quint8)NULL;
if (hdUSB!=NULL){Close();}

ftStatus = FT_ListDevices(BufPtrs,&numDev,FT_LIST_ALL|FT_OPEN_BY_DESCRIPTION);        // поиск устройств по описанию
if((FT_SUCCESS(ftStatus))||(ftStatus==FT_DEVICE_NOT_OPENED)){
    itm.clear();
    for (int i=0;i<(int)numDev;i++)
        {
        QString st;
        if(!strcmp(BufPtrs[i],"Debag_for_UDZ")){
            st.append(BufPtrs[i]);st.append(" "+QString::number(n++));
            numDevice = i; // наше устройство
            counDevice++;
            }else{
            st.append(BufPtrs[i]);
            }
       itm.append(st);
       }
    }else Return=retErr;   // ошибка
//
if(counDevice!=1)Return=retChoice; //  нет устройств или найдено не одно
            else Return=retOk;
emit signalSearchUsb(itm,numDevice,Return);
return Return;
}
/* Search device no parametr*/
quint16 ftdiChip::SearchDevice()
{
FT_STATUS ftStatus;
DWORD numDev; // колличество USB устройств
quint8 counDevice = (quint8)NULL;
//if (hdUSB!=NULL){Close();}
ftStatus = FT_ListDevices(BufPtrs,&numDev,FT_LIST_ALL|FT_OPEN_BY_DESCRIPTION);        // поиск устройств по описанию

if((FT_SUCCESS(ftStatus))||(ftStatus==FT_DEVICE_NOT_OPENED)){
    for (int i=0;i<(int)numDev;i++)
        {
        if(!strcmp(BufPtrs[i],"Debag_for_UDZ")){
            numDevice = i;counDevice++;} // наше устройство запоминаем
        }
    }
    else{
        return retErr; // ошибка отработки команды
        }
if(counDevice!=1)return retChoice; // найдено более одного устройства
return retOk;
}
/* set parameters device USB*/
bool ftdiChip::setParameterDevice(const QStringList &prm)
{
if(prm.count()<5)return false;
parameter.setTimeDelay(prm.at(4).toLong());
parameter.setSpeed(prm.at(5).toLong());
parameter.setDataBit(prm.at(6).toInt());
parameter.setStopBit(prm.at(7).toInt());
parameter.setParity(prm.at(8).toInt());
return true;
}
/*Close USB*/
void ftdiChip::Close()
{
FT_Close(hdUSB);
hdUSB = NULL;
}
/* Open USB */
quint16 ftdiChip::Open()
{
FT_STATUS ftStatus;

if (hdUSB!=NULL){Close();}

ftStatus=FT_Open(numDevice,&hdUSB);
    if (ftStatus!=FT_OK){
        emit signalStatusError(tr("Error open USB"),true);
        return retErr;
        }
ftStatus=FT_SetUSBParameters(hdUSB,1024,1024);
if (ftStatus!=FT_OK){// Error setting buffer sizes USB
    emit signalStatusError(tr("Error setting buffer sizes USB"),true);
    return retErr;
    }
ftStatus = FT_SetTimeouts(hdUSB,10,1);
if(ftStatus != FT_OK){
  return retErr;
  }
return retOk;
}
/* Read buffer USB*/
quint16 ftdiChip::Read(QByteArray &buff)
{
FT_STATUS ftStatus;
DWORD  numout,RxBytes;// RxBuf=NULL, TxBuf=NULL, EventStat;

//ftStatus=FT_GetStatus(hdUSB,&RxBuf,&TxBuf, &EventStat);
//do{
  ftStatus=FT_GetQueueStatus(hdUSB,&RxBytes);
  if(ftStatus==FT_OK){
    if(RxBytes>0){ //чтение только если в приёмном буфере что то есть
      QByteArray tmpbuff(RxBytes,0x00);
      ftStatus=FT_Read(hdUSB,tmpbuff.data(),RxBytes,&numout);
      if(ftStatus!=FT_OK)return retErr;
      buff.append(tmpbuff);
      }
      else return retBusyDevice;
    }else return retErr;
  //}while(RxBytes);
return retOk;
}
/*Writing data to USB */
quint16 ftdiChip::Write(void *pdata, DWORD leng, DWORD *nMin)
{
FT_STATUS ftStatus;
DWORD dwRx=0,dwTx=0,dwEvent=0;

ftStatus=FT_GetStatus(hdUSB,&dwRx,&dwTx,&dwEvent);
if (ftStatus!=FT_OK){
    return retErr;
    }
if ((4096-dwTx)>=(DWORD)leng){// запись, если в выходном буфере есть место
    ftStatus=FT_Write(hdUSB,pdata,leng,nMin);
    if(ftStatus!=FT_OK){
        emit signalStatusError(tr("Error writing data to USB"),true);
        return retErr;
        }
    }
ftStatus=FT_GetStatus(hdUSB,&dwRx,&dwTx,&dwEvent);
if (ftStatus!=FT_OK){
    return retErr;
    }
return retOk;
}
/* Writing data 8 bit to USB */
quint16 ftdiChip::Bit8Write(QByteArray &buff, quint16 repet)
{
FT_STATUS ftStatus;
DWORD nmin,i=0,leng = buff.size();
char *pdata = buff.data();

while (i<repet)
    {
    ftStatus=Write(pdata,leng,&nmin);
    if (ftStatus==retErr){
            return retErr;// error
            }
    i++;
    }
return retOk;
}
/* Writing data 9 bit to USB */
quint16 ftdiChip::Bit9Write(QByteArray &buff, quint16 repet)
{
    FT_STATUS ftStatus;
    DWORD nmin,i=0,leng = buff.size();
    char *pdata = buff.data();

    while (i<repet)
            {
            ftStatus=FT_SetDataCharacteristics(hdUSB,FT_BITS_8,FT_STOP_BITS_1,FT_PARITY_MARK);    //  устанавливает FT_PARITY_MARK FT_PARITY_ODD
            if (ftStatus!=FT_OK){
                    emit signalStatusError(tr("USB write error status"),true);
                    return retErr;// error
                    }
            ftStatus=Write(pdata,1,&nmin);
            if (ftStatus==retErr){
                    return retErr;// error
                    }
            ftStatus=FT_SetDataCharacteristics(hdUSB,FT_BITS_8,FT_STOP_BITS_1,FT_PARITY_SPACE);   //  сбрасывает бит FT_PARITY_SPACE FT_PARITY_EVEN
            if (ftStatus!=FT_OK){
                    emit signalStatusError(tr("USB write error status"),true);
                    return retErr;// error
                    }
            pdata++;
            ftStatus=Write(pdata,(leng-1),&nmin);
            if (ftStatus==retErr){
                    return retErr;// error
                    }
            i++;
            }

    return retOk;
}
/* SET PARAMETR USB*/
quint16 ftdiChip::setParametersUSB(quint64 Speed, quint8 DataBit, quint8 StopBit, quint8 Parity)
{
FT_STATUS ftStatus;

ftStatus=FT_SetBaudRate(hdUSB,Speed);
if (ftStatus){
    emit signalStatusError(tr("Error setting parameters <speed FTDI>"),true);
    return retErr;
    }
parameter.setSpeed(Speed);
ftStatus=FT_SetDataCharacteristics(hdUSB,(UCHAR)DataBit,(UCHAR)StopBit,(UCHAR)Parity);
if (ftStatus!=FT_OK){
    emit signalStatusError(tr("Error setting parameters <databit FTDI>"),true);
    return retErr;
    }
parameter.setDataBit(DataBit);
parameter.setStopBit(StopBit);
parameter.setParity(Parity);
emit signalStatusOk(tr("Parameters is successful!"));
return retOk;
}
/* Writing data to EEPROM FTDI */
quint16 ftdiChip::on_EEPROM()
{
FT_STATUS ftStatus;
FT_PROGRAM_DATA ftData;
char ManufacturerBuf[32];
char ManufacturerIdBuf[16];
char DescriptionBuf[64];
char SerialNumberBuf[16];

memset(&ftData,0,sizeof(ftData));
ftData.Signature1 = 0x00000000;
ftData.Signature2 = 0xffffffff;
ftData.Version = 0x00000002;    // EEPROM structure with FT232R extensions  */
ftData.Manufacturer = ManufacturerBuf;
ftData.ManufacturerId = ManufacturerIdBuf;
ftData.Description = DescriptionBuf;
ftData.SerialNumber = SerialNumberBuf;
ftStatus = Open();
if(ftStatus==retOk){
    ftStatus = FT_EE_Read(hdUSB, &ftData);
    if (ftStatus != FT_OK) {
        emit signalStatusError(tr("Failed to open USB port"),true);
        return retErr;
        }
    strcpy(ftData.Manufacturer,"ELINTEL");
    strcpy(ftData.ManufacturerId,"FT");
    strcpy(ftData.Description,"Debag_for_UDZ");
    strcpy(ftData.SerialNumber,"FT000001");
    ftData.Cbus0 = 0x0A;
    ftData.Cbus1 = 0x0A;
    ftData.Cbus2 = 0x00;
    ftData.Cbus3 = 0x0A;
    ftData.Cbus4 = 0x01;
ftStatus = FT_EE_Program(hdUSB, &ftData);
    if (ftStatus != FT_OK){
        emit signalStatusError(tr("Error writing flash"),true);
        return retErr;
        }
    emit signalStatusOk(tr("Flash is successfully"));
    return retOk;
    }
emit signalStatusError(tr("Error writing flash"),true);
return retErr;
}
/*Set number USB*/
void ftdiChip::on_SetNumberUSB(const quint64 number)
{
    numDevice = number;
}
/*Checksum of the received packet*/
ANWR_PROTOCOL::RETURN_ANSWER ftdiChip::CheckingReceivedPacket(const QByteArray &bS)
{
if(bS.isEmpty())return ANWR_PROTOCOL::retNoAnsError;
QByteArray cmd = bS.simplified();
if(cmd.contains("#")){
    if(cmd[1]=='>')return ANWR_PROTOCOL::retOK;
    if(cmd[1]=='?')return ANWR_PROTOCOL::retIncorData;
    }
return ANWR_PROTOCOL::retError;
}
