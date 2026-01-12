#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QButtonGroup>
#include "socket.h"
#include "noticedialog.h"
#include "global.h"
#include "resultdialog.h"
#include "billsdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(UserInfo userInfo, QWidget *parent = nullptr);
    ~MainWindow();
private:
    void ensureWindowOnScreen(); // 解决切换不同分辨率屏幕时出现的显示异常，没有完全解决
public:
    void SetAvatar(QPixmap pix); //设置头像
    QIcon CreateColorDot(QColor color); // 画公告的等级圈
    void UploadAvatarToServer(const QPixmap& pix); // 上传头像
    void DownloadAvatarToServer(); // 下载头像
    void ShowContentDialog();
    void InitUi();
    void SetCursorStyle();
    void InitPersonalData(); // 初始化个人信息
    void AddChatMessage(QString sender, QString content, QString time, bool isPrivate);
    QJsonArray ImagesToBase64(const QStringList& imagePaths);
    void GetProcessProgress(); // 获取处理进度
    void SaveAvatarToLocal(const QPixmap& pix, const QString& sender = ""); // 保存头像到本地
    void GetBillsInfo(); // 获取用户账单信息
    void SetGroupSendBtn(); // 群发按钮格式
    void SetPrivateSendBtn(); // 私信按钮格式
private:
    void AddNoticeItem(const NoticeInfo& info); // 主界面显示公告标题等信息
    void ShowContentDialog(const NoticeInfo& info); // 展示公告详细内容
    void ToAddBill(BillInfo& info, bool checked); // 添加账单
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
signals:
    void logoutRequested();
private slots:
    void onUpdateAvatar(bool success, QString message, QString avatar, QString sender = "");
    void onUpdateNotice(bool success, QString message, const QList<NoticeInfo>& list);
    void onUpdateProgress(bool success, QString message, const QList<ReportInfo>& list);
    void onUpdateBills(bool success, QString message, const QList<BillInfo>& list);
    void onUpdatePayment(bool success, QString message, const QList<PaymentInfo>& list);
    void onUpdateInfo(bool success, QString message);
    void onDelivery(int type, QString content, int level, QStringList imagePaths, int status, QString time);
    void onPrivateSend(QString receiver);

    void on_changeAvatar_btn_clicked();
    void on_logOut_btn_clicked();
    void on_cancel_btn_clicked();
    void on_saveChange_btn_clicked();
    void on_send_btn_clicked();
    void on_dark_light_btn_clicked();

    void on_payAll_btn_clicked();

    void on_waterPay_btn_clicked(bool checked);

    void on_electPay_btn_clicked(bool checked);

    void on_gasPay_btn_clicked(bool checked);

    void on_parkPay_btn_clicked(bool checked);

    void on_propertyPay_btn_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    QPoint m_dragPosition; // 偏移
    Socket* m_sock;

    // 各种子窗口
    NoticeDialog* m_ntcDlg;
    ResultDialog* m_resDlg;
    BillsDialog* m_billDlg;
    UserInfo m_info;
    QString m_username;
    double m_balance; // 余额
    // 按钮组指针
    QButtonGroup* m_btnGroup;

    bool m_isBlack;
    // 花的钱
    double m_waterCost;
    double m_electCost;
    double m_gasCost;
    // 消耗的量
    double m_waterSpend;
    double m_electSpend;
    double m_gasSpend;

    QString m_parkDate;
    QString m_propertyDate;

    bool m_isPrivate; // 是不是私信
    QString m_receiver; // 私信给谁
};

#endif // MAINWINDOW_H
