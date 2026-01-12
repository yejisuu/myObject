#include "noticedialog.h"
#include "ui_noticedialog.h"

NoticeDialog::NoticeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NoticeDialog)
{
    ui->setupUi(this);

    connect(ui->close_btn, &QPushButton::clicked, this, &NoticeDialog::close);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // 背景透明
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->close_btn->setCursor(Qt::PointingHandCursor);
}

NoticeDialog::~NoticeDialog()
{
    delete ui;
}

void NoticeDialog::SetLogContent(const QString &title, const QString &content, const QString &sender, const QString &time, const QColor& color)
{
    ui->title_lb->setText(title);
    QString style = QString("QLabel#title_lb { color: %1; }").arg(color.name());
    ui->title_lb->setStyleSheet(style);
    ui->notice_tb->setText(content);
    QString send = "发送方:" + sender;
    ui->sendPeople_lb->setText(send);
    ui->sendPeople_lb->setStyleSheet("font-size: 11pt;");
    ui->sendTime_lb->setText(time);
    ui->sendTime_lb->setStyleSheet("font-size: 11pt;");
}

void NoticeDialog::mousePressEvent(QMouseEvent *event)
{
    // 只有按住鼠标左键才能拖动
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void NoticeDialog::mouseMoveEvent(QMouseEvent *event)
{
    // 只有按住左键移动才有效
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}
