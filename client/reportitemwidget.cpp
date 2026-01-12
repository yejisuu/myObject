#include "reportitemwidget.h"
#include "ui_reportitemwidget.h"
#include <QStyle>
#include <QSize>
#include <QHBoxLayout>

ReportItemWidget::ReportItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportItemWidget)
{
    ui->setupUi(this);

    // 透明背景，圆角
    this->setAttribute(Qt::WA_StyledBackground);
    this->setCursor(Qt::PointingHandCursor);
    this->setStyleSheet(
        "QWidget#ReportItemWidget { "
        "    background-color: transparent;"
        "}"
        "QLabel { color: #ffffff; font-size: 14px; }" // 默认文字颜色
    );
}

ReportItemWidget::~ReportItemWidget()
{
    delete ui;
}

void ReportItemWidget::setData(const ReportInfo &info)
{
    QString typeStr = (info.type == 1) ? "【报修】" : "【投诉】";
    ui->event_lb->setText(typeStr);
    QString contentShort = info.content.left(21) + (info.content.length() > 21 ? "..." : "");
    ui->title_lb->setText(contentShort);
    // 设置状态颜色和文字
    QString color;
    QString statusStr;
    switch (info.status) {
    case 0:
        color = "#FF4D4F"; // 红
        statusStr = "待处理";
        break;
    case 1:
        color = "#FAAD14"; // 黄
        statusStr = "处理中";
        break;
    case 2:
        color = "#52C41A"; // 绿
        statusStr = "已处理";
        break;
    default:
        color = "#999999";
        statusStr = "未知";
        break;
    }

    ui->status_lb->setText(statusStr);
    ui->status_lb->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));
}

void ReportItemWidget::setAdminData(const ReportInfo &info)
{
    QString color;
    QString statusStr;
    QString typeStr = (info.type == 1) ? "【报修】" : "【投诉】";
    ui->event_lb->setText(typeStr);
    switch (info.level) {
    case 1:
        color = "#4CAF50";
        break;
    case 2:
        color = "#ADFF2F";
        break;
    case 3:
        color = "yellow";
        break;
    case 4:
        color = "orange";
        break;
    case 5:
        color = "red";
        break;
    default:
        color = "#999999";
        break;
    }
    ui->event_lb->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));
    ui->title_lb->setText(info.username);

    ui->title_lb->setStyleSheet("color: black;");
    switch (info.status) {
    case 0:
        color = "#FF4D4F"; // 红
        statusStr = "待处理";
        break;
    case 1:
        color = "#FAAD14"; // 黄
        statusStr = "处理中";
        break;
    case 2:
        color = "#52C41A"; // 绿
        statusStr = "已处理";
        break;
    default:
        color = "#999999";
        statusStr = "未知";
        break;
    }
    ui->status_lb->setText(statusStr);
    ui->status_lb->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color));

    QHBoxLayout *layout = new QHBoxLayout();
    ui->event_lb->setMinimumWidth(50);
    ui->event_lb->setMaximumWidth(80);
    ui->title_lb->setAlignment(Qt::AlignCenter);
    ui->status_lb->setMinimumWidth(50);
    ui->status_lb->setMaximumWidth(80);

    ui->event_lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->title_lb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->status_lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    // 添加到布局，并设置伸缩因子
    layout->addWidget(ui->event_lb, 0);     // 伸缩因子 0：不拉伸
    layout->addWidget(ui->title_lb, 1);   // 伸缩因子 1：拉伸填满剩余空间
    layout->addWidget(ui->status_lb, 0);    // 伸缩因子 0：不拉伸
    layout->setContentsMargins(0, 10, 0, 10);
    this->setLayout(layout);

}

void ReportItemWidget::setPayData(const QString &title, const QString &useInfo, double amount)
{
    this->setFixedSize(320, 40);

    ui->event_lb->setFixedSize(70, 30);
    ui->event_lb->setText(title);
    ui->event_lb->setStyleSheet(QString("color: #ffffff; font-weight: bold;"));
    ui->event_lb->move(10, 5);

    ui->title_lb->setFixedSize(170, 30);
    ui->title_lb->setText(useInfo);
    ui->title_lb->setStyleSheet(QString("color: #ffffff; font-weight: bold; padding: 2px 4px;"));
    ui->title_lb->move(70, 5);

    m_amount = amount;
    ui->status_lb->setFixedSize(70, 40);
    ui->status_lb->setText("¥" + QString::number(amount, 'f', 2));
    ui->status_lb->move(220, 10);
    ui->status_lb->setStyleSheet(QString("color: #ffffff; font-weight: bold;"));
    ui->status_lb->setAlignment(Qt::AlignLeft);
}

void ReportItemWidget::setPaymentData(const PaymentInfo &info)
{
    ui->event_lb->setStyleSheet(QString("color: #ffffff; font-weight: bold;"));
    ui->event_lb->setFixedSize(70, 20);
    ui->event_lb->setText(info.title);

    ui->title_lb->setFixedSize(220, 20);
    ui->title_lb->setText(info.time);
    ui->title_lb->move(165, 10);

    ui->status_lb->setFixedSize(80, 20);
    ui->status_lb->setText("¥" + QString::number(info.amount, 'f', 2));
    ui->status_lb->move(450, 10);
}

QString ReportItemWidget::getTitle()
{
    return ui->event_lb->text();
}

double ReportItemWidget::getAmount()
{
    return m_amount;
}

void ReportItemWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();  // 发射自定义信号
    }
}
