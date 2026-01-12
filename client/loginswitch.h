#ifndef LOGINSWITCH_H
#define LOGINSWITCH_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QHBoxLayout>

enum login_type{
    USER_LOGIN,
    ROOT_LOGIN
};

class LoginSwitch : public QWidget
{
    Q_OBJECT
public:
    explicit LoginSwitch(QWidget *parent = nullptr);
private:
    void SetIconSize(QPushButton* btn, QSize size);
    void SetIcon(QPushButton* btn, QIcon icon);
    void InitBtn();
signals:
    // 发出信号告诉主界面：0 居民，1 管理员
    void selectChanged(int index);
private slots:
    void onResidentClicked();
    void onAdminClicked();

private:
    QLabel* m_bgLabel;      // 背景条
    QLabel* m_slider;       // 滑块
    QPushButton* m_user;
    QPushButton* m_root;

    QPropertyAnimation *m_anim; // 动画

    login_type type;

    void updateIconStyle(int index);
};

#endif // LOGINSWITCH_H
