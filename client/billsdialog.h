#ifndef BILLSDIALOG_H
#define BILLSDIALOG_H

#include "ui_billsdialog.h"

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QList>
#include "resultdialog.h"

class BillsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BillsDialog(QWidget *parent = nullptr);

    // 添加一笔要缴的费用
    void AddBill(const QString& title, const QString& useInfo, double amount);
    // 删除要缴的费用
    void DelBill(const QString& title, double amount);
    // 保存余额
    void SetBalance(double balance){ m_balance = balance; };
    void InitUi();
signals:
    // 支付成功信号 (因为是批量，就不传具体金额了，主界面收到后统一清零欠费即可)
    void payAllSuccess(double cost, QList<PaymentInfo> list);

private slots:
    void onTimeOut(); // 模拟支付完成

    void on_zfb_btn_clicked();

    void on_wechat_btn_clicked();

    void on_balance_btn_clicked();

private:

    void showQrCode(const QString& imagePath);

private:
    Ui::BillsDialog* ui;
    ResultDialog* m_resDlg;
    // 数据
    double m_totalAmount;
    double m_balance;
    QTimer* m_checkTimer;
};


#endif // BILLSDIALOG_H





