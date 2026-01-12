#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsDropShadowEffect>
#include <QBuffer>
#include <QMovie>

#include "LoginSwitch.h"
#include "socket.h"
#include "registerdialog.h"
#include "retrievedialog.h"
#include "global.h"
#include "mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QDialog
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
    void ensureWindowOnScreen(); // 解决切换不同分辨率屏幕时出现的显示异常
public:
    void InitUI();
    QPixmap SetRoundAvatar(const QPixmap& pix, int radius, int borderWidth); //设置头像
    void SetCursorStyle();
    void InitLineStyle();
    void InitCloseStyle();
    void InitLoginSwitch(); // 初始化切换登录身份按钮
    void SaveConfig(); // 保存配置，用户名，密码，自动登录等信息
    void LoadConfig(); // 加载配置
    QString EncryptPwd(const QString& raw); // 保存的密码加密
    QString DecryptPwd(const QString& password); // 解密

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
signals:

private slots:
    void onLoginResult(bool success, QString message, QJsonObject data = {}); // 登录结果
    void onRegisterResult(bool success, QString message); // 注册结果
    void onRetrieveResult(bool success, QString message); // 找回结果
    void onChangeResult(bool success, QString message); // 修改结果
    void onRetrievePassword();
    void onRegisterUser();
    void onSendRegisterData(QString username, QString password, QString realname, QString phone);
    void onSendRetrieveData(QString username, QString realname, QString phone);
    void onSendChangePassword(QString username, QString password);
private slots:
    void on_login_btn_clicked(); // 用户登录
    void on_root_login_btn_clicked(); // 管理员登录
    void on_login_again_btn0_clicked(); // 重新登录
    void on_login_again_btn1_clicked(); // 重新登录
    void on_userPwd_edit_returnPressed();

    void on_rootPwd_edit_returnPressed();

private:
    Ui::Widget *ui;

    RetrieveDialog* m_retDlg;
    RegisterDialog* m_regDlg;
    Socket* m_sock;
    QPoint m_dragPosition; // 记录鼠标按下的相对位置
    bool m_isDragging = false; // 记录是否正在拖拽
    LoginSwitch *m_loginSwitch; // 用来初始化选择登录的角色
public:
    bool m_isUserLogin;
    UserInfo m_cacheUserInfo;

    MainWindow* m_mainWin = nullptr;

    QMovie* m_loadingMovie;
};
#endif // WIDGET_H
