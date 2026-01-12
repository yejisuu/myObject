#ifndef RETRIEVEDIALOG_H
#define RETRIEVEDIALOG_H

#include "ui_retrievedialog.h"

#include <QDialog>
#include <QStyle>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace Ui {
class RetrieveDialog;
}

class RetrieveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RetrieveDialog(QWidget *parent = nullptr);
    ~RetrieveDialog();
public:
    void InitUI();
    void SetCursorStyle();
    void CheckResult(bool success, QString message);
    void ChangeResult(bool success, QString message);
signals:
    void sendRetrieveData(QString username, QString realname, QString phone);
    void sendChangePassword(QString username, QString password);
private slots:
    void onUpdateDataStatus();
    void on_retrieve_btn_clicked();

    void on_return_btn1_clicked();

    void on_change_btn_clicked();

private:
    Ui::RetrieveDialog *ui;
    QString m_user;
    QString m_real;
    QString m_phone;
    QString m_password;
    QString m_password2;
};

#endif // RETRIEVEDIALOG_H
