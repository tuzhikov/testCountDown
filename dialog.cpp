#include "dialog.h"
#include "mainwindow.h"

Dialog::Dialog(QWidget *parent, QString text) :
    QDialog(parent),ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->lineEdit->setText(text);
    connect(ui->pushButtonExit,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->pushButtonSend,SIGNAL(clicked()),this,SLOT(on_SendMessage()));
    connect(ui->pushButtonCRC,SIGNAL(clicked()),this,SLOT(on_CheckCRC()));
}
Dialog::~Dialog()
{
    delete ui;
}
/*Check CRC*/
void Dialog::on_CheckCRC()
{
QByteArray crc,cmd = ui->lineEdit->text().toLocal8Bit();
if(cmd.contains("$")){
    crc = MainWindow::ShiftCRC8(cmd).toLocal8Bit();
    crc.prepend("$");
    int position = cmd.indexOf('$');
    cmd = cmd.left(position);
    cmd = cmd.append(crc);
    }
if(ui->checkBox->isChecked())cmd.append("\r");
ui->lineEdit->setText(cmd);
}


