#include "LoginSwitch.h"

LoginSwitch::LoginSwitch(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(100, 31);

    // 背景槽
    m_bgLabel = new QLabel(this);
    m_bgLabel->setGeometry(0, 0, 100, 31);
    m_bgLabel->setStyleSheet("background-color: transparent; border-radius: 15px; border: 1px solid rgba(255, 255, 255, 1);");

    // 滑块
    m_slider = new QLabel(this);
    m_slider->setGeometry(0, 0, 50, 31);
    m_slider->setStyleSheet("background-color: rgba(255, 255, 255, 100); border-radius: 15px;");

    // 初始化按钮
    InitBtn();
    connect(m_user, &QPushButton::clicked, this, &LoginSwitch::onResidentClicked);
    connect(m_root, &QPushButton::clicked, this, &LoginSwitch::onAdminClicked);

    type = ROOT_LOGIN;

    // 初始化动画
    m_anim = new QPropertyAnimation(m_slider, "pos");
    m_anim->setDuration(140); // 140毫秒滑过去
    m_anim->setEasingCurve(QEasingCurve::OutCubic); // 快进慢出的流畅效果

    m_user->setFocusPolicy(Qt::NoFocus);
    m_root->setFocusPolicy(Qt::NoFocus);
}

void LoginSwitch::SetIconSize(QPushButton* btn, QSize size){
    btn->setIconSize(size);
}

void LoginSwitch::SetIcon(QPushButton* btn, QIcon icon){
    btn->setIcon(icon);
}

void LoginSwitch::InitBtn(){
    m_user = new QPushButton("", this);
    m_root = new QPushButton("", this);
    SetIcon(m_user, QIcon(":/image/image/pressed_user.png"));
    SetIcon(m_root, QIcon(":/image/image/hover_root.png"));
    SetIconSize(m_user, QSize(20, 20));
    SetIconSize(m_root, QSize(20, 20));

    m_user->setGeometry(0, 0, 50, 30);
    m_root->setGeometry(50, 0, 50, 30);

    QString btnStyle = "QPushButton { background-color: transparent; border: none; }";
    m_user->setStyleSheet(btnStyle);
    m_root->setStyleSheet(btnStyle);
}

void LoginSwitch::onResidentClicked()
{
    if (type == USER_LOGIN) {
        m_anim->stop();
        m_anim->setEndValue(QPoint(0, 0));
        m_anim->start();

        type = ROOT_LOGIN;
        updateIconStyle(0);
        emit selectChanged(0); // 用户登录
    }
}

void LoginSwitch::onAdminClicked()
{
    if (type == ROOT_LOGIN) {
        m_anim->stop();
        m_anim->setEndValue(QPoint(50, 0));
        m_anim->start();
        type = USER_LOGIN;
        updateIconStyle(1);
        emit selectChanged(1); // 管理员登录
    }
}

void LoginSwitch::updateIconStyle(int index)
{
    if (index == 0) {
        SetIcon(m_user, QIcon(":/image/image/pressed_user.png"));
        SetIcon(m_root, QIcon(":/image/image/hover_root.png"));
    } else {
        SetIcon(m_user, QIcon(":/image/image/hover_user.png"));
        SetIcon(m_root, QIcon(":/image/image/pressed_root.png"));
    }
}
