#include "socket.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonArray>
#include <QVector>


Socket::Socket(QObject *parent)
    :QObject(parent)
{
    // http
    m_manager = new QNetworkAccessManager(this);

    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, &QTcpSocket::connected, this, &Socket::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Socket::onTcpReadyRead);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &Socket::onTcpDisconnected);
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error){
        qDebug() << "TCP 错误:" << error;
    });
}

Socket& Socket::getInstance()
{
    static Socket m_instance;
    return m_instance;
}

QString Socket::GenerateLoginSignature(const QString& username, const QString& password, qint64 timestamp, QString salt, QString string)
{
    QString raw = username + password + QString::number(timestamp) + salt + string;

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(raw.toUtf8());
    QString sign = hash.result().toHex(); // 把二进制MD5转成小写十六进制字符串
    return sign;
}

void Socket::SetServerConfig(const QString &url)
{
    m_url = url;
}

void Socket::SendJsonRequest(QJsonObject jsonRequest, const QString &action)
{
    QJsonDocument doc(jsonRequest);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact) + "\n";

    QUrl url(m_url + action);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //m_manager->post(request, jsonData);

    QNetworkReply* reply = m_manager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        this->onReplyFinished(reply);
    });

}

void Socket::SendLoginRequest(MSG_TYPE ask, const QString &username, const QString &password)
{
    // 获取时间戳
    qint64 timestamp = GetCurrentTimestamp();
    // 生成签名
    QString sign = GenerateLoginSignature(username, password, timestamp, SALT, SPECIAL_STRING);

    QJsonObject jsonRequest;
    jsonRequest["type"] = static_cast<int>(ask);
    jsonRequest["username"] = username;
    jsonRequest["timestamp"] = timestamp;
    jsonRequest["sign"] = sign; //签名

    SendJsonRequest(jsonRequest, "/login");
}

void Socket::SendRegisterRequest(const QString &username, const QString &password, const QString &realname, const QString &phone)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["password"] = password;
    jsonRequest["realname"] = realname;
    jsonRequest["phone"] = phone;

    SendJsonRequest(jsonRequest, "/register");
}

void Socket::SendRetrieveRequest(const QString &username, const QString &realname, const QString &phone)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["realname"] = realname;
    jsonRequest["phone"] = phone;

    SendJsonRequest(jsonRequest, "/retrieve");
}

void Socket::SendChangePwdRequest(const QString &username, const QString &password)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["password"] = password;

    SendJsonRequest(jsonRequest, "/changePwd");
}

void Socket::SendSaveAvatarRequest(const QString &username, const QString &avatar)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["avatar"] = avatar;

    SendJsonRequest(jsonRequest, "/saveAvatar");
}

void Socket::SendGetAvatarRequest(const QString &username, bool isSelf)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["isSelf"] = isSelf;

    SendJsonRequest(jsonRequest, "/getAvatar");
}

void Socket::SendGetNoticeRequest(const QString &username)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;

    SendJsonRequest(jsonRequest, "/getNotices");
}

void Socket::SendLogoutRequest(const QString &username)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;

    SendJsonRequest(jsonRequest, "/logout");
}

void Socket::SendChangeInfoRequest(UserInfo info)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = info.username;
    QJsonObject jsonData;
    jsonData["realname"] = info.realname;
    jsonData["phone"] = info.phone;
    jsonData["sex"] = info.sex;
    jsonData["building"] = info.building;
    jsonData["floor"] = info.floor;
    jsonData["housenum"] = info.housenum;

    jsonRequest["data"] = jsonData;

    SendJsonRequest(jsonRequest, "/changeInfo");
}

void Socket::SendDeliveryRequest(ReportInfo info)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = info.username;
    jsonRequest["content"] = info.content;
    jsonRequest["level"] = info.level;
    jsonRequest["status"] = info.status;
    jsonRequest["time"] = info.time;
    jsonRequest["type"] = info.type;
    jsonRequest["images"] = info.images;

    SendJsonRequest(jsonRequest, "/delivery");
}

void Socket::SendGetProgressRequest(const QString &username)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;

    SendJsonRequest(jsonRequest, "/getprogress");
}

void Socket::SendGetBillsInfoRequest(const QString &username)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;

    SendJsonRequest(jsonRequest, "/getBills");
}

void Socket::SendSetBillsInfoRequest(ChangeBillsInfo info)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = info.username;
    jsonRequest["balance"] = info.balance;
    jsonRequest["waterSpend"] = info.waterSpend;
    jsonRequest["electSpend"] = info.electSpend;
    jsonRequest["gasSpend"] = info.gasSpend;
    jsonRequest["parkSpend"] = info.parkSpend;
    jsonRequest["propertySpend"] = info.propertySpend;

    SendJsonRequest(jsonRequest, "/setBills");
}

void Socket::SendSetPaymentRecordRequest(const QString &username, QList<PaymentInfo> info)
{
    QJsonArray dataArray;
    for (const PaymentInfo& item : info) {
        QJsonObject itemObj;
        itemObj["amount"] = item.amount;
        itemObj["title"] = item.title;
        itemObj["time"] = item.time;
        dataArray.append(itemObj);
    }
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["items"] = dataArray;
    SendJsonRequest(jsonRequest, "/setPaymentList");
}

void Socket::SendGetPaymentRecordRequest(const QString &username)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;

    SendJsonRequest(jsonRequest, "/getPaymentList");
}

void Socket::SendPushNoticeRequest(NoticeInfo info)
{
    QJsonObject jsonRequest;
    jsonRequest["title"] = info.title;
    jsonRequest["content"] = info.content;
    jsonRequest["sender"] = info.sender;
    jsonRequest["time"] = info.time;
    int level;
    if(info.typeColor == "#FF4D4F") level = 3;
    else if(info.typeColor == "#FAAD14") level = 2;
    else level = 1;
    jsonRequest["level"] = level;
    SendJsonRequest(jsonRequest, "/pushNotice");
}

void Socket::SendAdminGetAllUserInfoRequest()
{
    QJsonObject jsonRequest;
    SendJsonRequest(jsonRequest, "/getAllInfo");
}

void Socket::SendAdminDeliveryInfoRequest(const QString &username, const QString &iamgesPath)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["iamgesPath"] = iamgesPath;
    SendJsonRequest(jsonRequest, "/getDeliveryInfo");
}

void Socket::SendUpdateDeliveryStatusRequest(const QString &username, const QString &content, int status)
{
    QJsonObject jsonRequest;
    jsonRequest["username"] = username;
    jsonRequest["content"] = content;
    jsonRequest["status"] = status;
    SendJsonRequest(jsonRequest, "/updateDeliveryStatus");
}

void Socket::ConnectChatServer(const QString username, const QString &ip, int port)
{
    // 先断开旧连接
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_tcpSocket->disconnectFromHost();
    }

    m_username = username;
    qDebug() << "正在连接聊天服务器..." << ip << port;
    m_tcpSocket->connectToHost(ip, port);
}

void Socket::SendGroupMessage(const QString &fromUser, const QString &content)
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "未连接聊天服务器，发送失败";
        return;
    }

    QJsonObject json;
    json["type"] = "group_msg";
    json["from"] = fromUser;
    json["text"] = content;
    json["time"] = QDateTime::currentDateTime().toString("M-d HH:mm:ss");

    // 转成 ByteArray 发送
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    m_tcpSocket->write(data);
    // 强制把缓冲区的数据推送到网络层
    m_tcpSocket->flush();
}

void Socket::SendPrivateMessage(const QString &fromUser, const QString &receiver, const QString &content)
{
    qDebug() << fromUser << "发送给" << receiver << content;

    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "未连接聊天服务器，发送失败";
        return;
    }

    QJsonObject json;
    json["type"] = "chat_msg";
    json["from"] = fromUser;
    json["text"] = content;
    json["receiver"] = receiver;
    json["time"] = QDateTime::currentDateTime().toString("M-d HH:mm:ss");

    // 转成 ByteArray 发送
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    m_tcpSocket->write(data);
    // 强制把缓冲区的数据推送到网络层
    m_tcpSocket->flush();
}

void Socket::DisconnectChatServer()
{
    m_tcpSocket->disconnectFromHost();
}

void Socket::onReplyFinished(QNetworkReply *reply)
{
    QString path = reply->request().url().path();
    // 检查网络状态
    if(reply->error() != QNetworkReply::NoError){
        QString errorMsg = "网络错误" + reply->errorString();
        qDebug() << errorMsg;
        // ssl错误提示
        if(reply->error() >= QNetworkReply::SslHandshakeFailedError){
            qDebug() << "ssl错误，检查证书是否有效/客户端是否缺少OpenSSL库";
        }
        emit loginResult(false, errorMsg);
        reply->deleteLater(); // 防止内存泄漏
        return;
    }
    // 读取服务器返回的数据
    QByteArray data = reply->readAll();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError || !doc.isObject()){
        qDebug() << "解析错误" << parseError.errorString();
        emit loginResult(false, "服务器回复格式错误");
    }
    else{
        QJsonObject replyObj = doc.object();
        if(path == "/login"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    QJsonObject userdata = replyObj["data"].toObject();
                    emit loginResult(true, message, userdata);
                }
                else{
                    emit loginResult(false, message);
                }
            }
        }
        else if(path == "/register"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                QString username = replyObj["username"].toString();
                if(status == "success"){
                    emit registerResult(true, message);
                    QString avatarPath = ":/image/image/default_avatar.png";
                    QPixmap pix(avatarPath);
                    QByteArray bytes;
                    QBuffer buffer(&bytes);
                    buffer.open(QIODevice::WriteOnly);
                    pix.save(&buffer, "PNG");
                    QString base64 = bytes.toBase64();
                    this->SendSaveAvatarRequest(username, base64);
                }
                else{
                    emit registerResult(false, message);
                }
            }
        }
        else if(path == "/retrieve"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    emit retrieveResult(true, message);
                }
                else{
                    emit retrieveResult(false, message);
                }
            }
        }
        else if(path == "/changePwd"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    emit changeResult(true, message);
                }
                else{
                    emit changeResult(false, message);
                }
            }
        }
        else if(path == "/saveAvatar"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString message = replyObj["message"].toString();
                qDebug() << message;
            }
        }
        else if(path == "/getAvatar"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                QString avatar = replyObj["avatar"].toString();
                QString sender = replyObj["sender"].toString();
                if(status == "success"){
                    if(sender != "") {
                        emit avatarResult(true, message, avatar, sender);
                    }
                    else emit avatarResult(true, message, avatar);
                }
                else{
                    emit avatarResult(false, message, avatar);
                }
            }
        }
        else if(path == "/getNotices"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    QList<NoticeInfo> list;
                    QJsonArray arr = replyObj["data"].toArray();
                    // 循环解析 JSON 数组
                    for (const QJsonValue &val : arr) {
                        QJsonObject obj = val.toObject();
                        NoticeInfo info;

                        info.title   = obj["title"].toString();
                        info.content = obj["content"].toString();
                        info.sender  = obj["sender"].toString();
                        info.time    = obj["time"].toString(); // 比如 "2023-12-12 10:00:00"

                        // 数据库存的是 level (1, 2, 3)，我们要转成 QColor
                        int level = obj["level"].toInt();
                        switch (level) {
                        case 3: // 紧急
                            info.typeColor = QColor("#FF4D4F"); // 红色
                            break;
                        case 2: // 重要
                            info.typeColor = QColor("#FAAD14"); // 黄色/橙色
                            break;
                        case 1: // 普通
                        default:
                            info.typeColor = QColor("#52C41A"); // 绿色
                            break;
                        }
                        // 添加到列表
                        list.append(info);
                    }
                    emit noticeResult(true, message, list);
                }
                else{
                    emit noticeResult(false, message, QList<NoticeInfo>());
                }
            }
        }
        else if(path == "/changeInfo"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    emit infoResult(true, message);
                }
                else{
                    emit infoResult(false, message);
                }
            }
        }
        else if(path == "/getprogress"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                QString role = replyObj["role"].toString();
                if(status == "success"){
                    if(role == "user"){
                        QList<ReportInfo> list;
                        QJsonArray arr = replyObj["data"].toArray();
                        // 循环解析json数组
                        for (const QJsonValue &val : arr) {
                            QJsonObject obj = val.toObject();
                            ReportInfo info;
                            info.status   = obj["status"].toInt();
                            info.content = obj["content"].toString();
                            info.type  = obj["type"].toInt();
                            list.append(info);
                        }
                        emit progressResult(true, message, list);
                    }
                    else{
                        // 管理员读取数据
                        QList<ReportInfo> list;
                        QJsonArray arr = replyObj["data"].toArray();
                        // 循环解析json数组
                        for (const QJsonValue &val : arr) {
                            QJsonObject obj = val.toObject();
                            ReportInfo info;
                            info.status   = obj["status"].toInt();
                            info.content = obj["content"].toString();
                            info.type  = obj["type"].toInt();
                            info.username = obj["username"].toString();
                            info.level = obj["level"].toInt();
                            info.imagesPath = obj["iamgesPath"].toString();

                            list.append(info);
                        }
                        emit adminProgressResult(true, message, list);
                    }
                }
                else{
                    if(role == "user") emit progressResult(true, message, QList<ReportInfo>());
                    else emit adminProgressResult(false, message, QList<ReportInfo>());
                }
            }
        }
        else if(path == "/getBills"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();

                if(status == "success"){
                    QList<BillInfo> list;
                    QJsonArray arr = replyObj["data"].toArray();
                    // 循环解析json数组
                    for (const QJsonValue &val : arr) {
                        QJsonObject obj = val.toObject();
                        BillInfo info;
                        QVector<double> vctList;
                        info.title   = obj["title"].toString();
                        info.spend = obj["spend"].toDouble();
                        info.subInfo  = obj["subInfo"].toString();
                        if (obj.contains("dataList") && obj["dataList"].isArray()) {
                            QJsonArray jsonList = obj["dataList"].toArray();
                            // 遍历 JSON 数组，转成 double 存入 QVector
                            for (const QJsonValue& numVal : jsonList) {
                                vctList.append(numVal.toDouble());
                            }
                        }
                        info.dataList = vctList;
                        list.append(info);
                    }
                    emit billsResult(true, message, list);
                }
                else{
                    emit billsResult(false, message, QList<BillInfo>());
                }
            }
        }
        else if(path == "/getPaymentList"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    QList<PaymentInfo> list;
                    QJsonArray arr = replyObj["data"].toArray();
                    // 循环解析json数组
                    for (const QJsonValue& val : arr) {
                        QJsonObject obj = val.toObject();
                        PaymentInfo info;
                        info.title   = obj["title"].toString();
                        info.time = obj["time"].toString();
                        info.amount  = obj["amount"].toDouble();
                        list.append(info);
                    }
                    emit paymentResult(true, message, list);
                }
                else{
                    emit paymentResult(false, message, QList<PaymentInfo>());
                }
            }
        }
        else if(path == "/getAllInfo"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    QList<UserAllInfo> list;
                    QJsonArray arr = replyObj["data"].toArray();
                    for(const QJsonValue& val : arr){
                        QJsonObject obj = val.toObject();
                        UserAllInfo info;
                        info.username = obj["username"].toString();
                        info.realname = obj["realname"].toString();
                        info.phone = obj["phone"].toString();
                        info.building = obj["building"].toString();
                        info.floor = obj["floor"].toInt();
                        info.housenum = obj["housenum"].toInt();
                        info.balance = obj["balance"].toDouble();
                        info.water_spend = obj["water_spend"].toDouble();
                        info.water_info = obj["water_info"].toString();
                        info.elect_spend = obj["elect_spend"].toDouble();
                        info.elect_info = obj["elect_info"].toString();
                        info.gas_spend = obj["gas_spend"].toDouble();
                        info.gas_info = obj["gas_info"].toString();
                        info.temperature = obj["temperature"].toDouble();
                        info.area = obj["area"].toString();
                        info.parking_date = obj["parking_date"].toString();
                        info.property_date = obj["property_date"].toString();
                        list.append(info);
                    }
                    allUserInfoResult(true, message, list);
                }
                else{
                    allUserInfoResult(false, message, QList<UserAllInfo>());
                }
            }
        }
        else if(path == "/getDeliveryInfo"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    QVector<QByteArray> imageData;
                    QString phone = replyObj["phone"].toString();
                    QString address = replyObj["address"].toString();
                    QJsonArray imgs = replyObj["images"].toArray();
                    for(const QJsonValue& img : imgs){
                        QJsonObject obj = img.toObject();
                        QString imagestr = obj["image"].toString();
                        QByteArray imgBytes = QByteArray::fromBase64(imagestr.toUtf8());
                        imageData.push_back(imgBytes);
                    }
                    deliveryInfoResult(true, message, phone, address, imageData);
                }
                else{
                    deliveryInfoResult(false, message, "", "",  QVector<QByteArray>());
                }
            }
        }
        else if(path == "/updateDeliveryStatus"){
            if(replyObj.contains("status") && replyObj["status"].isString()){
                QString status = replyObj["status"].toString();
                QString message = replyObj["message"].toString();
                if(status == "success"){
                    updateDeliveryStatusResult(true, message);
                }
                else{
                    updateDeliveryStatusResult(false, message);
                }
            }
        }

    }

    reply->deleteLater();
}

void Socket::onTcpReadyRead()
{
    // 读取所有数据
    QByteArray data = m_tcpSocket->readAll();
    // 解析json
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "收到非json数据或解析错误";
        return;
    }

    QJsonObject root = doc.object();
    QString type = root["type"].toString();
    if (type == "group_msg") {
        QString from = root["from"].toString();
        QString text = root["text"].toString();
        QString time = root["time"].toString();
        // 检查本地有没有此人的头像
        QString appPath = QCoreApplication::applicationDirPath();
        QString imgPath = appPath + "/cache/avatar_" + from + ".png";
        QFileInfo fileInfo(imgPath);
        // 没有，发送请求头像
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            // 触发http请求，请求头像
            this->SendGetAvatarRequest(from, false);
        }
        else{
            emit chatMessageReceived(from, text, time);
        }
    }
    else if(type == "pushNotice"){
        this->SendGetNoticeRequest("anyone");
    }
    else if(type == "updateDelivery"){
        this->SendGetProgressRequest(m_username);
    }
    else if(type == "pushDelivery"){
        this->SendGetProgressRequest("");
    }
    else if(type == "chat_msg"){
        QString from = root["from"].toString();
        QString text = root["text"].toString();
        QString time = root["time"].toString();
        // 检查本地有没有此人的头像
        QString appPath = QCoreApplication::applicationDirPath();
        QString imgPath = appPath + "/cache/avatar_" + from + ".png";
        QFileInfo fileInfo(imgPath);
        // 没有，发送请求头像
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            // 触发http请求，请求头像
            this->SendGetAvatarRequest(from, false);
        }
        else{
            // 有就触发消息到达信号
            emit chatMessageReceived(from, text, time, true);
        }
    }
}

void Socket::onTcpDisconnected()
{
    qDebug() << "聊天服务器连接已断开";
}

void Socket::onTcpConnected()
{
    if (!m_username.isEmpty()) {
        QJsonObject json;
        json["type"] = "chat_login";
        json["username"] = m_username;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson(QJsonDocument::Compact);
        data.append('\n');

        m_tcpSocket->write(data);
        m_tcpSocket->flush();

        qDebug() << "已发送tcp认证包:" << data;
    }
}







