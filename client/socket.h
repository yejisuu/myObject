#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include "global.h"
#include <QJsonObject>
#include <QPixmap>
#include <QBuffer>
#include <vector>

class Socket : public QObject
{
    Q_OBJECT
private:
    explicit Socket(QObject* parent = nullptr);
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
public:
    static Socket& getInstance();
public:
    // 生成登录签名(MD5)
    QString GenerateLoginSignature(const QString& username, const QString& password, qint64 timestamp, QString salt, QString string);
    // 设置服务器API地址
    void SetServerConfig(const QString& url);
    // json请求
    void SendJsonRequest(QJsonObject jsonRequest, const QString& action);
    // 发送登录请求
    void SendLoginRequest(MSG_TYPE who, const QString& username, const QString& password);
    // 发送注册请求
    void SendRegisterRequest(const QString& username, const QString& password, const QString& realname, const QString& phone);
    // 发送找回请求
    void SendRetrieveRequest(const QString& username, const QString& realname, const QString& phone);
    // 发送修改请求
    void SendChangePwdRequest(const QString& username, const QString& password);
    // 发送保存头像请求
    void SendSaveAvatarRequest(const QString& username, const QString& avatar);
    // 发送获取头像请求
    void SendGetAvatarRequest(const QString& username, bool isSelf);
    // 发送获取公告列表请求
    void SendGetNoticeRequest(const QString& username);
    // 发送退出登录请求
    void SendLogoutRequest(const QString& username);
    // 发送更改个人信息请求
    void SendChangeInfoRequest(UserInfo info);
    // 发送投递报修投诉请求
    void SendDeliveryRequest(ReportInfo info);
    // 发送处理进度请求
    void SendGetProgressRequest(const QString& username);
    // 发送获取账单请求
    void SendGetBillsInfoRequest(const QString& username);
    // 发送更新账单请求
    void SendSetBillsInfoRequest(ChangeBillsInfo info);
    // 发送记录缴费请求
    void SendSetPaymentRecordRequest(const QString& username, QList<PaymentInfo> info);
    // 发送获取记录缴费请求
    void SendGetPaymentRecordRequest(const QString& username);
    // 发送推送公告请求
    void SendPushNoticeRequest(NoticeInfo info);
    // 发送管理员索取所有用户信息请求
    void SendAdminGetAllUserInfoRequest();
    // 发送管理员获取报修投诉的详细信息请求
    void SendAdminDeliveryInfoRequest(const QString& username, const QString& iamgesPath);
    // 更新投诉报修处理状态
    void SendUpdateDeliveryStatusRequest(const QString& username, const QString& content, int status);
public:
    // 连接到聊天服务器,登录成功后调用
    void ConnectChatServer(const QString username, const QString& ip, int port);
    // 发送群聊消息
    void SendGroupMessage(const QString& fromUser, const QString& content);
    // 发送私信
    void SendPrivateMessage(const QString& fromUser, const QString& receiver, const QString& content);
    // 断开连接
    void DisconnectChatServer();
public:
    // 生成当前时间戳(秒)
    qint64 GetCurrentTimestamp(){ return QDateTime::currentSecsSinceEpoch(); }
private:
    // 处理服务器发送来的信息
signals:
    void loginResult(bool success, QString message, QJsonObject data = {}); // 是否登录上
    void registerResult(bool success, QString message); // 注册结果
    void retrieveResult(bool success, QString message); // 找回结果
    void changeResult(bool success, QString message); // 修改结果
    void avatarResult(bool success, QString message, QString avatar, QString sender = ""); // 获取头像结果
    void noticeResult(bool success, QString message, const QList<NoticeInfo>& list); // 返回的公告列表
    void infoResult(bool success, QString message); // 修改个人信息结果
    void progressResult(bool success, QString message, const QList<ReportInfo>& list); // 处理进度结果
    void adminProgressResult(bool success, QString message, const QList<ReportInfo>& list); // 管理员获取投诉报修信息
    void billsResult(bool success, QString message, const QList<BillInfo>& list); // 账单获取结果
    void paymentResult(bool success, QString message, const QList<PaymentInfo>& list); // 缴费记录获取结果
    void chatMessageReceived(QString fromUser, QString content, QString time, bool isPrivate = false); // 收到的消息
    void allUserInfoResult(bool success, QString message, const QList<UserAllInfo>& list); // 获取到的所有用户信息
    void deliveryInfoResult(bool success, QString message, QString phone, QString address, const QVector<QByteArray>& imgsBytes); // 投诉报修详细信息
    void updateDeliveryStatusResult(bool success, QString message);
public slots:
    // 接收所有http回复
    void onReplyFinished(QNetworkReply* reply);
    // tcp读取数据
    void onTcpReadyRead();
    // tcp断开连接
    void onTcpDisconnected();
    // tcp连接成功
    void onTcpConnected();
private:
    QNetworkAccessManager* m_manager;
    QString m_url;
    static Socket m_instance;
    QTcpSocket* m_tcpSocket; // 用于发送聊天消息
    QString m_username;
};

#endif // SOCKET_H
