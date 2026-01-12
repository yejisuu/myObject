#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "ui_registerdialog.h"

#include <QDialog>
#include <QStyle>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();
public:
    void InitUI();
    void SetCursorStyle();
    void CheckResult(bool success, QString message);
signals:
    void sendRegisterData(QString username, QString password, QString realname, QString phone);
private slots:
    void on_register_btn_clicked();
    void onUpdateDataStatus();
    void on_return_btn_clicked();

private:
    Ui::RegisterDialog *ui;
    QString m_user;
    QString m_pass;
    QString m_real;
    QString m_phone;
};

#endif // REGISTERDIALOG_H
