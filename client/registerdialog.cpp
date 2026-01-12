#include "registerdialog.h"


RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFocus();

    ui->reg_stackedWidget->setCurrentIndex(0);
    InitUI();
    SetCursorStyle();
    // 关闭按钮
    ui->close_btn->setIconSize(QSize(20, 20));
    connect(ui->close_btn, &QPushButton::clicked, this, &QWidget::close);

    connect(ui->username_edit, &QLineEdit::textChanged, this, &RegisterDialog::onUpdateDataStatus);
    connect(ui->password_edit, &QLineEdit::textChanged, this, &RegisterDialog::onUpdateDataStatus);
    connect(ui->realname_edit, &QLineEdit::textChanged, this, &RegisterDialog::onUpdateDataStatus);
    connect(ui->phone_edit, &QLineEdit::textChanged, this, &RegisterDialog::onUpdateDataStatus);
    // 设置只能输入1开头的数字
    QRegularExpression rxPhone("^1[0-9]*$");
    QValidator *validatorPhone = new QRegularExpressionValidator(rxPhone, this);
    ui->phone_edit->setValidator(validatorPhone);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::InitUI()
{
    ui->username_edit->setPlaceholderText("请输入用户名");
    ui->username_edit->setAlignment(Qt::AlignCenter); // 居中
    ui->password_edit->setPlaceholderText("请输入密码");
    ui->password_edit->setAlignment(Qt::AlignCenter);
    ui->password_edit->setEchoMode(QLineEdit::Password); // 设置为密码模式
    ui->realname_edit->setPlaceholderText("请输入真实姓名");
    ui->realname_edit->setAlignment(Qt::AlignCenter);
    ui->phone_edit->setPlaceholderText("请输入手机号");
    ui->phone_edit->setAlignment(Qt::AlignCenter);
    ui->phone_edit->setMaxLength(11);

    ui->register_lb->setProperty("styleType", "text");
    ui->register_lb->style()->unpolish(ui->register_lb);
    ui->register_lb->style()->polish(ui->register_lb);
    ui->text_lb->setProperty("styleType", "text");
    ui->text_lb->style()->unpolish(ui->text_lb);
    ui->text_lb->style()->polish(ui->text_lb);

    ui->register_btn->setEnabled(false);
}

void RegisterDialog::SetCursorStyle()
{
    ui->username_edit->setCursor(Qt::PointingHandCursor);
    ui->password_edit->setCursor(Qt::PointingHandCursor);
    ui->realname_edit->setCursor(Qt::PointingHandCursor);
    ui->phone_edit->setCursor(Qt::PointingHandCursor);
    ui->close_btn->setCursor(Qt::PointingHandCursor);
    ui->return_btn->setCursor(Qt::PointingHandCursor);
}

void RegisterDialog::CheckResult(bool success, QString message)
{
    ui->reg_stackedWidget->setCurrentIndex(1);
    if(success){
        ui->text_lb->setText(message);
    }
    else{
        ui->text_lb->setText(message);
    }
}

void RegisterDialog::on_register_btn_clicked()
{
    m_user = ui->username_edit->text();
    m_pass = ui->password_edit->text();
    m_real = ui->realname_edit->text();
    m_phone = ui->phone_edit->text();

    emit sendRegisterData(m_user, m_pass, m_real, m_phone);
}

void RegisterDialog::onUpdateDataStatus()
{
    // trimmed()去掉首尾空格，防止用户只输空格
    bool userOk = !ui->username_edit->text().trimmed().isEmpty();
    bool passOk = !ui->password_edit->text().trimmed().isEmpty();
    bool realOk = !ui->realname_edit->text().trimmed().isEmpty();
    bool phoneOk = !(ui->phone_edit->text().trimmed().length() != 11);

    if(userOk && passOk && realOk && phoneOk){
        ui->register_btn->setEnabled(true);
        ui->register_btn->setCursor(Qt::PointingHandCursor);
    }
    else{
        ui->register_btn->setEnabled(false);
    }
}


void RegisterDialog::on_return_btn_clicked()
{
    this->close();
}

