#include "resultdialog.h"
#include "ui_resultdialog.h"
#include <QStyle>

ResultDialog::ResultDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResultDialog)
{
    ui->setupUi(this);

    connect(ui->close_btn, &QPushButton::clicked, this, &ResultDialog::close);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // 背景透明
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->close_btn->setCursor(Qt::PointingHandCursor);

    ui->message_lb->setProperty("styleType", "text");
    ui->message_lb->style()->unpolish(ui->message_lb);
    ui->message_lb->style()->polish(ui->message_lb);
    ui->message_lb->setWordWrap(true); // 自动换行
    ui->message_lb->setAlignment(Qt::AlignCenter | Qt::AlignTop); // 让换行的文字顶部对齐
}

ResultDialog::~ResultDialog()
{
    delete ui;
}

void ResultDialog::SetMessage(QString message)
{
    ui->message_lb->setText(message);
}
