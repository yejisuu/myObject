#include "leftbubble.h"
#include "ui_leftbubble.h"

LeftBubble::LeftBubble(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LeftBubble)
{
    ui->setupUi(this);
    ui->Lmsg_lb->setType(BubbleLabel::Left);
    ui->Lavatar_lb->move(0, 12);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->info_lb->setStyleSheet("color: #ffffff");
    // 绑定点击头像的信号
    connect(ui->Lavatar_lb, &clickablelabel::clicked, this, &LeftBubble::onClicked);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

LeftBubble::~LeftBubble()
{
    delete ui;
}

void LeftBubble::SetTime(const QString &time)
{
    ui->info_lb->setText(time);
}

void LeftBubble::SetSender(const QString &sender)
{
    m_username = sender;
}

void LeftBubble::SetText(const QString &text)
{
    ui->Lmsg_lb->setText(text);

    // 定义气泡的最大宽度
    int maxBubbleWidth = 140;

    // 计算文字如果不换行，实际有多宽
    QFontMetrics fm(ui->Lmsg_lb->font());
    int textRealWidth = fm.horizontalAdvance(text);
    // 判断逻辑
    if (textRealWidth > maxBubbleWidth) {
        // 设置固定宽度
        ui->Lmsg_lb->setFixedWidth(maxBubbleWidth);
    } else {
        ui->Lmsg_lb->setMinimumWidth(0);
        ui->Lmsg_lb->setMaximumWidth(QWIDGETSIZE_MAX);
    }
}

void LeftBubble::SetAvatar(const QString &path)
{
    // 简单的设置图片逻辑
    ui->Lavatar_lb->setPixmap(QPixmap(path));
    ui->Lavatar_lb->setScaledContents(true);
}

void LeftBubble::SetContentColor(QColor color)
{
    QString style = QString("color: %1").arg(color.name());
    ui->Lmsg_lb->setStyleSheet(style);
}

void LeftBubble::onClicked()
{
    // 弹出Qmenu
    QMenu menu(this);
    QAction* sendMsg = menu.addAction("发送私信");
    sendMsg->setIcon(QIcon(":/image/image/private_send.png"));
    QPoint pos = ui->Lavatar_lb->mapToGlobal(QPoint(0, ui->Lavatar_lb->height()));
    QAction* choose = menu.exec(pos);
    if(choose == sendMsg){
        emit privateSend(m_username);
    }
}
