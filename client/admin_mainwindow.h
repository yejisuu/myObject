#ifndef ADMIN_MAINWINDOW_H
#define ADMIN_MAINWINDOW_H

#include <QMainWindow>
#include "global.h"
#include "socket.h"
#include "adminreportdialog.h"
#include <QListWidget>

namespace Ui {
class Admin_MainWindow;
}

class Admin_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Admin_MainWindow(QWidget *parent = nullptr);
    ~Admin_MainWindow();
public:
    void InitUi();
    void AddTableRow(UserAllInfo info);
private slots:
    void onProgressResult(bool success, QString message, const QList<ReportInfo>& list);
    void onallUserInfoResult(bool success, QString message, const QList<UserAllInfo> &list);
    void onDeliveryInfoResult(bool success, QString message, QString phone, QString address, const QVector<QByteArray>& imgsBytes);
    void onUpdateDelivery(int status);
    void on_send_btn_clicked();

signals:
    void myItemClicked(QListWidgetItem* item);
public:
    void onItemClicked(QListWidgetItem* item);

private:
    Ui::Admin_MainWindow *ui;
    AdminReportDialog* m_rptDlg;
    Socket* m_sock;

    QString m_content;
    int m_type;
    int m_status;
    QString m_username;
};

#endif // ADMIN_MAINWINDOW_H
