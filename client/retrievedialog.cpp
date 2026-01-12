#include "retrievedialog.h"


RetrieveDialog::RetrieveDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RetrieveDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFocus();

    ui->ret_stackedWidget->setCurrentIndex(0);
    InitUI();
    SetCursorStyle();
    // 关闭按钮
    ui->close_btn1->setIconSize(QSize(20, 20));
    connect(ui->close_btn1, &QPushButton::clicked, this, &QWidget::close);

    connect(ui->username_edit1, &QLineEdit::textChanged, this, &RetrieveDialog::onUpdateDataStatus);
    connect(ui->realname_edit1, &QLineEdit::textChanged, this, &RetrieveDialog::onUpdateDataStatus);
    connect(ui->phone_edit1, &QLineEdit::textChanged, this, &RetrieveDialog::onUpdateDataStatus);
    // 设置只能输入1开头的数字
    QRegularExpression rxPhone("^1[0-9]*$");
    QValidator *validatorPhone = new QRegularExpressionValidator(rxPhone, this);
    ui->phone_edit1->setValidator(validatorPhone);
}

RetrieveDialog::~RetrieveDialog()
{
    delete ui;
}

void RetrieveDialog::InitUI()
{
    ui->username_edit1->setPlaceholderText("请输入用户名");
    ui->username_edit1->setAlignment(Qt::AlignCenter); // 居中
    ui->realname_edit1->setPlaceholderText("请输入真实姓名");
    ui->realname_edit1->setAlignment(Qt::AlignCenter);
    ui->phone_edit1->setPlaceholderText("请输入手机号");
    ui->phone_edit1->setAlignment(Qt::AlignCenter);
    ui->phone_edit1->setMaxLength(11);

    ui->retrieve_lb->setProperty("styleType", "text");
    ui->retrieve_lb->style()->unpolish(ui->retrieve_lb);
    ui->retrieve_lb->style()->polish(ui->retrieve_lb);
    ui->text_lb1->setProperty("styleType", "text");
    ui->text_lb1->style()->unpolish(ui->text_lb1);
    ui->text_lb1->style()->polish(ui->text_lb1);
    ui->error_lb->setProperty("styleType", "text");
    ui->error_lb->style()->unpolish(ui->error_lb);
    ui->error_lb->style()->polish(ui->error_lb);

    ui->newPwd_edit->setPlaceholderText("请输入密码");
    ui->newPwd_edit->setAlignment(Qt::AlignCenter);
    ui->newPwd_edit->setEchoMode(QLineEdit::Password); // 设置为密码模式

    ui->again_edit->setPlaceholderText("请确认密码");
    ui->again_edit->setAlignment(Qt::AlignCenter);
    ui->again_edit->setEchoMode(QLineEdit::Password);

    ui->retrieve_btn->setEnabled(false);
}

void RetrieveDialog::SetCursorStyle()
{
    ui->username_edit1->setCursor(Qt::PointingHandCursor);
    ui->realname_edit1->setCursor(Qt::PointingHandCursor);
    ui->phone_edit1->setCursor(Qt::PointingHandCursor);
    ui->close_btn1->setCursor(Qt::PointingHandCursor);
    ui->return_btn1->setCursor(Qt::PointingHandCursor);
    ui->change_btn->setCursor(Qt::PointingHandCursor);
}

void RetrieveDialog::CheckResult(bool success, QString message)
{
    ui->ret_stackedWidget->setCurrentIndex(1);
    if(success){
        this->setFocus();
        ui->retrieve_lb->setText("设置新密码");
        ui->stackedWidget->setCurrentIndex(0);
    }
    else{
        ui->stackedWidget->setCurrentIndex(1);
        ui->text_lb1->setText(message);
    }
}

void RetrieveDialog::ChangeResult(bool success, QString message)
{
    ui->text_lb1->setText(message);
    ui->stackedWidget->setCurrentIndex(1);
}

void RetrieveDialog::onUpdateDataStatus()
{
    // trimmed()去掉首尾空格，防止用户只输空格
    bool userOk = !ui->username_edit1->text().trimmed().isEmpty();
    bool realOk = !ui->realname_edit1->text().trimmed().isEmpty();
    bool phoneOk = !(ui->phone_edit1->text().trimmed().length() != 11);

    if(userOk && realOk && phoneOk){
        ui->retrieve_btn->setEnabled(true);
        ui->retrieve_btn->setCursor(Qt::PointingHandCursor);
    }
    else{
        ui->retrieve_btn->setEnabled(false);
    }
}

void RetrieveDialog::on_retrieve_btn_clicked()
{
    m_user = ui->username_edit1->text();
    m_real = ui->realname_edit1->text();
    m_phone = ui->phone_edit1->text();

    emit sendRetrieveData(m_user, m_real, m_phone);
}

void RetrieveDialog::on_return_btn1_clicked()
{
    this->close();
}

void RetrieveDialog::on_change_btn_clicked()
{
    m_password = ui->newPwd_edit->text();
    m_password2 = ui->again_edit->text();
    if(m_password != m_password2){
        ui->error_lb->setText("两次密码输入不一致，请重新输入!");
        return;
    }
    // 调用修改
    sendChangePassword(m_user, m_password);
}

