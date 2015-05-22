#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timerCount(new QTimer(parent)),
    usb(new ftdiChip(parent)),
    ProgressEnd(false)
{
    ui->setupUi(this);
    // Group menu
    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(ui->actionNormal);
    alignmentGroup->addAction(ui->actionTerminal);
    QActionGroup *alignmentGroup_HEX = new QActionGroup(this);
    alignmentGroup_HEX->addAction(ui->actionDEC);
    alignmentGroup_HEX->addAction(ui->actionHEX);
    /*Connections list*/
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(on_About()));
    connect(ui->pushButtonStop,SIGNAL(clicked()),this,SLOT(on_StopButton()));
    connect(ui->pushButtonStart,SIGNAL(clicked()),this,SLOT(on_StartButton()));
    connect(ui->pushButtonWinManual,SIGNAL(clicked()),this,SLOT(on_Manual()));
    connect(timerCount,SIGNAL(timeout()),this,SLOT(on_Progress()));
    /*Connection usb module*/
    connect(usb,SIGNAL(signalSendMessage(bool,QByteArray,QColor)),this,
            SLOT(on_GetMessageOutTextEdit(bool,QByteArray,QColor)));
    connect(this,SIGNAL(slPrmUSB(quint64,quint64,quint8,quint8,quint8)),
            usb,SLOT(on_ParameterLink(quint64,quint64,quint8,quint8,quint8)));
    connect(ui->actionUSB_RS485,SIGNAL(triggered()),usb,SLOT(on_SearchDevice()));
    connect(usb,SIGNAL(signalSearchUsb(QStringList&,quint16,quint16)),this,
            SLOT(on_USB_choice(QStringList&,quint16,quint16)));
    connect(this,SIGNAL(slNumberUSB(quint64)),usb,SLOT(on_SetNumberUSB(quint64)));
    connect(usb,SIGNAL(signalStatusError(QString)),this,SLOT(on_SetMessageStatusBarError(QString)));
    connect(usb,SIGNAL(signalStatusOk(QString)),this,SLOT(on_SetMessageStatusOk(QString)));
    connect(usb,SIGNAL(signalStop()),this,SLOT(on_StopButton()));
    /*Create progress bar*/
    progress_status = new QProgressBar(this);
    progress_status->setVisible(false);
    progress_status->setTextVisible(false);
    connect(usb,SIGNAL(signalProgressRange(int,int)),progress_status,SLOT(setRange(int,int)),Qt::DirectConnection);
    connect(usb,SIGNAL(signalProgressValue(int,bool)),this,SLOT(on_setValueProgress(int,bool)),Qt::DirectConnection);
    ui->statusBar->addPermanentWidget(progress_status,0);
    /*Set message in status bar*/
    QLabel *p = new QLabel(this);
    p->setText(tr("version 1.2"));
    p->setOpenExternalLinks(true);
    ui->statusBar->addPermanentWidget(p);
    ui->statusBar->showMessage(tr("Test CountDown"),10000);
    /*XML parameter*/
    TParserXML parserXml;
    parserXml.OpenXML("devices.xml",strParamXML);
    ShowParametersForm(strParamXML);
    /*Read setting in register*/
    readSettings();
    /*Create Style*/
    CreateStyle();
    /*Activate USB*/
    usb->setParameterDevice(strParamXML);
    if(usb->SearchDevice()!=ftdiChip::retOk){
        usb->on_SearchDevice();
        }

}

MainWindow::~MainWindow()
{
    delete ui;
}
/*Create Style*/
void MainWindow::CreateStyle()
{
/*set Style Sweet the windows and button*/
QFile styleFile(":/QSS/QSS/style_files001.qss");
bool result = styleFile.open(QFile::ReadOnly);
if(result){
    QByteArray style = styleFile.readAll();
    qApp->setStyleSheet(style);
    }
}
/*write setting registry*/
void MainWindow::writeSettings()
{
QSettings settings("Software LLC Elintel", "Setting CountDown StPetersburg");
settings.beginGroup("MainForm");
settings.setValue("geometry", saveGeometry());
settings.endGroup();
}
/*read setting registry*/
void MainWindow::readSettings()
{
QSettings settings("Software LLC Elintel", "Setting CountDown StPetersburg");
settings.beginGroup("MainForm");
restoreGeometry(settings.value("geometry").toByteArray());
settings.endGroup();
}
/*Close Event */
void MainWindow::closeEvent(QCloseEvent *event)
{
writeSettings();
event->accept();
}
/* Coller LCD */
void MainWindow::SegmentStyleLCD(QColor color)
{
ui->lcdNumber->setSegmentStyle(QLCDNumber::Filled);
QPalette palette = ui->lcdNumber->palette();
palette.setColor(QPalette::WindowText,color);
ui->lcdNumber->setPalette(palette);
}
/*creat windows about */
void MainWindow::on_About(void)
{
QMessageBox::about(this, tr("About the program, a countdown"),
                   tr(
                       "<h2> Test countdown board 'TOO' version 1.2</h2>"
                       "<p>Co., Ltd. Elintel <a href=\"http://www.elintel.ru/\">www.elintel.ru</a> </p>"
                       "<p>The program countdown."
                      ));
}
/* start button */
void MainWindow::on_StartButton()
{
emit slPrmUSB(strParamXML.at(4).toLong(),
              strParamXML.at(5).toLong(),
              strParamXML.at(6).toInt(),
              strParamXML.at(7).toInt(),
              strParamXML.at(8).toInt());
if(ui->tabWidget->currentIndex()){ // page auto?
    QString cmddata;
    CollectDataToSen(cmddata,Qt::red);
    QByteArray cmd = cmddata.toLocal8Bit();
    usb->sendMessage(cmd);
    }else{
    timerCount->setInterval(1000);
    timerCount->start();
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonStop->setEnabled(true);
    ui->tabWidget->setEnabled(false);
    ui->frameAdr->setEnabled(false);
    }
}
/* stop button */
void MainWindow::on_StopButton()
{
usb->stopMessage();
timerCount->stop();
ui->pushButtonStop->setEnabled(false);
ui->pushButtonStart->setEnabled(true);
ui->tabWidget->setEnabled(true);
ui->frameAdr->setEnabled(true);
ProgressEnd = true;
}
/*CountDown board */
bool MainWindow::showCountDownBiard(QTime &time)
{
ui->lcdNumber->display(time.toString("m:ss"));
if((time.minute()==0)&&(time.second()==0)) return true;
time=time.addSecs(-1);
return false;
}
/* machine  */
void MainWindow::on_Progress()
{
static STATELIGHT CountDownSate;
static QTime timeCounter;
QString cmddata;
QByteArray cmd;
switch((int)CountDownSate)
    {
    case stRed:
        ProgressEnd = false;
        timeCounter = ui->timeEditRed->time();
        SegmentStyleLCD(Qt::red);
        CollectDataToSen(cmddata,Qt::red);
        cmd = cmddata.toLocal8Bit();
        usb->sendMessage(cmd);
        CountDownSate = stActionRed;
    case stActionRed:
        if(showCountDownBiard(timeCounter))CountDownSate = stGreen;
        if(ProgressEnd)CountDownSate = stRed;
        return;
    case stGreen:
        timeCounter = ui->timeEditGreen->time();
        SegmentStyleLCD(Qt::green);
        CollectDataToSen(cmddata,Qt::green);
        cmd = cmddata.toLocal8Bit();
        usb->sendMessage(cmd);
        CountDownSate = stActionGreen;
    case stActionGreen:
        if(showCountDownBiard(timeCounter))CountDownSate = stRed;
        if(ProgressEnd)CountDownSate = stRed;
        return;
    case stEndProgress:
    default:
        ProgressEnd = false;
        CountDownSate = stRed;
        return;
    }
}
/*Get Message Output*/
void MainWindow::on_GetMessageOutTextEdit(const bool direction, const QByteArray &st, const QColor &color)
{
QByteArray str;
if(direction)str.append("READ> ");
        else str.append("SEND> ");
ui->textEditLog->setTextColor(color);
//str.append(st.toHex());
str.append(st);
str = str.trimmed();
ui->textEditLog->append(str);
}
/*Set messages in status bar*/
void MainWindow::on_SetMessageStatusBarError(const QString &message)
{
QPalette palette = ui->statusBar->palette();
palette.setColor(QPalette::WindowText,Qt::red);
ui->statusBar->setPalette(palette);
ui->statusBar->showMessage(message,10000);
QMessageBox::critical(this,tr("Message Error"),message);
}
/*Set messages in status bar Ok*/
void MainWindow::on_SetMessageStatusOk(const QString &message)
{
QPalette palette = ui->statusBar->palette();
palette.setColor(QPalette::WindowText,Qt::darkBlue);
ui->statusBar->setPalette(palette);
ui->statusBar->showMessage(message,10000);
}
/* Choice USB */
void MainWindow:: on_USB_choice(QStringList& itm,quint16 index,quint16 Result)
{
if((Result==ftdiChip::retOk)||(Result==ftdiChip::retChoice)){
    bool ok;
    QString item = QInputDialog::getItem(this, tr("Identification USB devices"),
                                        tr("Available USB device"),itm, index, false, &ok);
    if (ok && !item.isEmpty()){ // Ok
        for(int i=0;i<itm.size();i++)
            {
            if(!item.compare(item,itm.at(i))){
                emit slNumberUSB(i);
                break;
                }
            }
        }
    }else{
    QMessageBox::critical(this,tr("USB failed"),tr("USB/RS485 not found!"));
    }
}
/*set data progress*/
void MainWindow::on_setValueProgress(const int value, const bool visible)
{
progress_status->setVisible(visible);
progress_status->setValue(value);
}
/* show parameter form */
void MainWindow::ShowParametersForm(const QStringList &strParm)
{
ui->spinBoxAdr->setValue(1);
ui->timeEditRed->setTime(QTime::fromString(strParm.at(0),"m:ss"));
ui->spinBoxRed->setValue(strParm.at(1).toInt());
ui->timeEditGreen->setTime(QTime::fromString(strParm.at(2),"m:ss"));
ui->spinBoxGreen->setValue(strParm.at(3).toInt());
// LCD display
SegmentStyleLCD(Qt::green);
ui->lcdNumber->display(strParm.at(2));
//visibled button
ui->pushButtonStop->setEnabled(false);
}
/*Create manual windows */
void MainWindow::on_Manual()
{
Dialog *dialog = new Dialog(this);
connect(dialog,SIGNAL(SendMessage(QString)),this,SLOT(on_SendUsbDebug(QString)),Qt::DirectConnection);
dialog->exec();
delete dialog;
}
/*Send USB Debug*/
void MainWindow::on_SendUsbDebug(QString cmd)
{
QByteArray cm = cmd.toLocal8Bit();
usb->sendMessage(cm);
}
/* checked HEX or DEC */
QString MainWindow::StringHEX(const QString str)const
{
QString stres;

if(str!=NULL){
    if(ui->actionHEX->isChecked()){
        int res = str.toInt();
        return stres = "0x"+QString::number(res,16).toUpper();
        }
    }
return str;
}
/*Converter time all sec*/
QString MainWindow::ConvertTimeToSec(const QTime time)const
{
int sec = (time.minute()*60)+time.second();
return QString::number(sec);
}
/*collect data to widget page auto*/
QString MainWindow::CollectDataToWidgetPageAuto(const int indexcolor)const
{
QString message;
if(indexcolor == Qt::green){
    message ="g "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ConvertTimeToSec(ui->timeEditGreen->time()))
             +" "+StringHEX(ui->spinBoxGreen->text());
    }else{
    message ="w "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ConvertTimeToSec(ui->timeEditRed->time()))
             +" "+StringHEX(ui->spinBoxRed->text());
    }
return message;
}
/*collect data to widget page test*/
QString MainWindow::CollectDataToWidgetPageTest()const
{
QString message;
// test command
if(ui->radioButtonTestOne->isChecked()){
    message ="w "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ConvertTimeToSec(ui->timeEditRedTest->time()))
                    +" "+StringHEX(ui->spinBoxRedTest->text());
    }
if(ui->radioButtonTestTwo->isChecked()){
    message ="g "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ConvertTimeToSec(ui->timeEditGreenTest->time()))
                    +" "+StringHEX(ui->spinBoxGreenTest->text());
    }
if(ui->radioButtonTestThree->isChecked()){
    message ="x "+StringHEX(ui->spinBoxAdr->text());
    }
if(ui->radioButtonTestFour->isChecked()){
    message ="h "+StringHEX(ui->spinBoxAdr->text());
    }
if(ui->radioButtonTestFive->isChecked()){
    message ="v "+StringHEX(ui->spinBoxAdr->text());
    }
if(ui->radioButtonTestSix->isChecked()){
    message ="d "+StringHEX(ui->spinBoxAdr->text());
    }
return message;
}
/*collect data to widget page service*/
QString MainWindow::CollectDataToWidgetPageService()const
{
QString message;
// service command
if(ui->radioButtonSrvGetAdr->isChecked()){// get adress
    message ="a "+StringHEX(ui->spinBoxAdr->text());
    }
if(ui->radioButtonSvrSetAdr->isChecked()){// set adress
    message ="a "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ui->spinBoxAdrSrvNew->text())
                                                      +" "+StringHEX(ui->spinBoxAdrSrvNew->text());
    }
if(ui->radioButtonSrvTwo->isChecked()){   // set blinked
    message ="f "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ui->spinBoxBlinkSrv->text());
    }
if(ui->radioButtonSrvThree->isChecked()){// set current
    message ="y "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ui->spinBoxCurrentSrv->text());
    }
if(ui->radioButtonSrvFour->isChecked()){  // set light
    message ="b "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ui->spinBoxBrightSrv->text());
    }
if(ui->radioButtonSrvTest1->isChecked()){ // test 1
    message ="t "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ui->spinBoxTestSvr->text());
    }
if(ui->radioButtonSrvTest2->isChecked()){  // test 2
    message ="T "+StringHEX(ui->spinBoxAdr->text())+" "+StringHEX(ui->spinBoxTestSvr->text());
    }
return message;
}
/*collect data to widget*/
QString MainWindow::CollectDataToWidget(const int indexpage,const int indexcolor)const
{
QString message;

switch(indexpage)
    {
    case 0:
        message=CollectDataToWidgetPageAuto(indexcolor);
        break;
    case 1:
        message=CollectDataToWidgetPageTest();
        break;
    case 2:
        message=CollectDataToWidgetPageService();
        break;
    }
return message;
}
// Collect data to send USB
void MainWindow::CollectDataToSen(QString &str,const int cdcolor)
{
//quint16 CS;
str.clear();
if(ui->actionNormal->isChecked())str = "#";
                            else str = "!";
str +=CollectDataToWidget(ui->tabWidget->currentIndex(),cdcolor);
if(ui->actionNormal->isChecked()){
    str.append(" $");
    str.append(ShiftCRC8(str.toLocal8Bit()));
    }
str  = str.simplified();
str.append("\r");
}
/*ShiftCRC8*/
QString MainWindow::ShiftCRC8(const QByteArray &data)
{
quint8 CalculCRC = 0;
quint16 leng = data.size();
const quint8 *pMess = (quint8*)(data.data());

while(leng--)
    {
    if((CalculCRC+(*pMess))>255){
        CalculCRC+=*pMess;
        CalculCRC+=1;
        }else{
        CalculCRC+=*pMess;
        }
    CalculCRC = (CalculCRC << 1) | (CalculCRC >> 7);
    pMess++;
    if(*pMess=='$')break;
    }
QString result;
if(CalculCRC<0x10){
    result = "0"+QString::number(CalculCRC,16).toUpper();
    }else{
    result = QString::number(CalculCRC,16).toUpper();
    }
return result;
}
/*shift left*/
/*unsigned int left_shift(unsigned int n,unsigned int k) {
 unsigned int i,bit;
 for (i=0; i<k; i++) {
  bit=n&0x8000?1:0;
  n<<=1;
  n|=bit;
 }
 return n;
}*/

/*
unsigned char Crc8(unsigned char *pcBlock, unsigned int len)
{
    unsigned char crc = 0xFF;
    unsigned int i;

    while (len--)
    {
        crc ^= *pcBlock++;

        for (i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }

    return crc;
}
*/
