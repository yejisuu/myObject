#include "adminreportdialog.h"
#include "ui_adminreportdialog.h"

AdminReportDialog::AdminReportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdminReportDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("投诉/报修事务");
    InitUi();
}

AdminReportDialog::~AdminReportDialog()
{
    delete ui;
}

void AdminReportDialog::SetDeliveryInfo(const FinalDeliveryInfo &info)
{
    QString typeStr = (info.type == 1) ? "【报修】" : "【投诉】";
    ui->type_lb->setText(typeStr);
    ui->content_edit->setText(info.content);
    ui->username_lb->setText(info.username);
    ui->phone_lb->setText(info.phone);
    ui->address_lb->setText(info.address);
    if(info.status == 0) ui->deal_btn->setEnabled(true);// 去处理可用
    else if(info.status == 1) ui->Processed_btn->setEnabled(true); // 完成可用

    // 解析图片
    int count = 0;
    QVector<QByteArray> list = info.imgs;
    for(const QByteArray& imgData : list){
        QPixmap pixmap;
        if(pixmap.loadFromData(imgData)){
            m_labelArray[count]->setPixmap(pixmap.scaled(91, 91, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_labelArray[count]->setScaledContents(false);
            m_labelArray[count]->setAlignment(Qt::AlignCenter);
        }
        count++;
    }
}

void AdminReportDialog::InitUi()
{
    ui->deal_btn->setEnabled(false);
    ui->Processed_btn->setEnabled(false);

    m_labelArray.push_back(ui->image_lb1);
    m_labelArray.push_back(ui->image_lb2);
    m_labelArray.push_back(ui->image_lb3);
    m_labelArray.push_back(ui->image_lb4);
    m_labelArray.push_back(ui->image_lb5);

    ui->deal_btn->setCursor(Qt::PointingHandCursor);
    ui->Processed_btn->setCursor(Qt::PointingHandCursor);
}

void AdminReportDialog::on_deal_btn_clicked()
{
    emit updateDelivery(1);
    this->close();
}

void AdminReportDialog::on_Processed_btn_clicked()
{
    emit updateDelivery(2);
    this->close();
}

