#include "billsdialog.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QDebug>

BillsDialog::BillsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BillsDialog)
    , m_totalAmount(0.0)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->setAttribute(Qt::WA_TranslucentBackground);

    InitUi();
    ui->pay_widget->SetUi();

    m_checkTimer = new QTimer(this);
    m_checkTimer->setSingleShot(true);
    connect(m_checkTimer, &QTimer::timeout, this, &BillsDialog::onTimeOut);

    connect(ui->close_btn, &QPushButton::clicked, this, &QWidget::close);
}

void BillsDialog::AddBill(const QString &title, const QString& useInfo, double amount)
{
    ui->pay_widget->addPayOptItem(title, useInfo, amount);
    m_totalAmount += amount;

    ui->total_lb->setText(QString("合计: ¥%1").arg(m_totalAmount, 0, 'f', 2));
    if(m_totalAmount > m_balance) ui->balance_btn->setEnabled(false);
    ui->balance_btn->show();
    ui->wechat_btn->show();
    ui->zfb_btn->show();
}

void BillsDialog::DelBill(const QString &title, double amount)
{
    ui->pay_widget->delPayOptItem(title);
    m_totalAmount -= amount;
    ui->total_lb->setText(QString("合计: ¥%1").arg(m_totalAmount, 0, 'f', 2));
    if(m_totalAmount > m_balance) ui->balance_btn->setEnabled(false);
    if(ui->pay_widget->GetItemCount() == 0){
        ui->balance_btn->hide();
        ui->wechat_btn->hide();
        ui->zfb_btn->hide();
    }
}

void BillsDialog::showQrCode(const QString &imagePath)
{
    // 隐藏按钮，显示二维码
    ui->balance_btn->hide();
    ui->wechat_btn->hide();
    ui->zfb_btn->hide();
    ui->total_lb->hide();
    ui->qr_lb->show();
    ui->status_lb->show();
    // 加载图片
    QPixmap pix(imagePath);
    if (pix.isNull()) {
        ui->qr_lb->setText("二维码加载失败\n(请检查资源路径)");
        ui->qr_lb->setStyleSheet("background: #eeeeee; border: 1px solid #cccccc;");
    } else {
        ui->qr_lb->setPixmap(pix);
    }

    // 倒计时
    ui->status_lb->setText(QString("请扫码支付总额: ¥%1\n支付结果确认中...").arg(m_totalAmount, 0, 'f', 2));
    ui->status_lb->setStyleSheet("color: #ff6600; font-weight: bold;");
    m_checkTimer->start(3000);
}

void BillsDialog::onTimeOut()
{
    ui->status_lb->setText("支付成功!");
    ui->status_lb->setStyleSheet("color: green; font-size: 11pt; font-weight: bold;");

    QList<PaymentInfo> list = ui->pay_widget->GetPayInfoList();
    emit payAllSuccess(0, list);

    m_resDlg = new ResultDialog(this);
    m_resDlg->setWindowModality(Qt::ApplicationModal);
    m_resDlg->SetMessage("所有账单已完成缴费!");
    m_resDlg->exec();
}

void BillsDialog::InitUi()
{
    ui->balance_btn->hide();
    ui->wechat_btn->hide();
    ui->zfb_btn->hide();

    ui->close_btn->setCursor(Qt::PointingHandCursor);
    ui->balance_btn->setCursor(Qt::PointingHandCursor);
    ui->wechat_btn->setCursor(Qt::PointingHandCursor);
    ui->zfb_btn->setCursor(Qt::PointingHandCursor);

    ui->total_lb->setStyleSheet("font-size: 20px; font-weight: bold; color: #ff4500;");
    ui->total_lb->setAlignment(Qt::AlignRight);

    // 二维码区域
    ui->qr_lb->setScaledContents(true); // 自动缩放
    ui->qr_lb->setStyleSheet("border: 1px solid #cccccc;");
    ui->qr_lb->hide();

    ui->status_lb->setAlignment(Qt::AlignCenter);
    ui->status_lb->setStyleSheet("color: #ffffff;");

}

void BillsDialog::on_zfb_btn_clicked()
{
    showQrCode(":/image/image/zfbQr.jpg");
}

void BillsDialog::on_wechat_btn_clicked()
{
    showQrCode(":/image/image/wechatQr.jpg");
}

void BillsDialog::on_balance_btn_clicked()
{
    ui->balance_btn->hide();
    ui->wechat_btn->hide();
    ui->zfb_btn->hide();
    ui->status_lb->setText("支付成功!");
    ui->status_lb->move(0, 300);
    ui->status_lb->setStyleSheet("color: green; font-size: 15pt; font-weight: bold;");

    QList<PaymentInfo> list = ui->pay_widget->GetPayInfoList();
    emit payAllSuccess(m_totalAmount, list);

    m_resDlg = new ResultDialog(this);
    m_resDlg->setWindowModality(Qt::ApplicationModal);
    m_resDlg->SetMessage("所有账单已完成缴费!");
    m_resDlg->exec();

    ui->pay_widget->clearAll();
    //ui->balance_btn->show();
    //ui->wechat_btn->show();
    //ui->zfb_btn->show();
    ui->total_lb->setText("合计: ¥0.00");
    ui->status_lb->setText("");
    m_totalAmount = 0;
}

