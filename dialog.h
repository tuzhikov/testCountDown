#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "ui_dialog.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
Q_OBJECT

    Ui::Dialog *ui;
public:
    explicit Dialog(QWidget *parent = 0,QString text="#x 255 $95");
    ~Dialog();
public slots:
    void on_SendMessage(){emit SendMessage(ui->lineEdit->text());}
    void on_CheckCRC();
signals:
    void SendMessage(QString);
};

#endif // DIALOG_H
