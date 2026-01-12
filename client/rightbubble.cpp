#include "rightbubble.h"
#include "ui_rightbubble.h"

RightBubble::RightBubble(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RightBubble)
{
    ui->setupUi(this);
    ui->Rmsg_lb->setType(BubbleLabel::Right);
    ui->horizontalLayout->setAlignment(ui->Ravatar_lb, Qt::AlignTop);
    this->setAttribute(Qt::WA_TranslucentBackground);
}

RightBubble::~RightBubble()
{
    delete ui;
}

void RightBubble::SetText(const QString& text)
{
    ui->Rmsg_lb->setText(text);

    // 定义气泡的最大宽度
    int maxBubbleWidth = 140;

    // 计算文字如果不换行，实际有多宽
    QFontMetrics fm(ui->Rmsg_lb->font());
    int textRealWidth = fm.horizontalAdvance(text);

    // 判断逻辑
    if (textRealWidth > maxBubbleWidth) {
        ui->Rmsg_lb->setFixedWidth(maxBubbleWidth);
    } else {
        ui->Rmsg_lb->setMinimumWidth(0);
        ui->Rmsg_lb->setMaximumWidth(QWIDGETSIZE_MAX);
    }
}

void RightBubble::SetAvatar(const QString& path)
{
    ui->Ravatar_lb->setPixmap(QPixmap(path));
    ui->Ravatar_lb->setScaledContents(true);
}

void RightBubble::SetContentColor(QColor color)
{
    QString style = QString("color: %1").arg(color.name());
    ui->Rmsg_lb->setStyleSheet(style);
}
