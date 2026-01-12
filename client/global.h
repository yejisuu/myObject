#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QColor>
#include <QMetaType>
#include <QDateTime>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>

const QString SALT = "WANNABE_YYDS"; // 盐
const QString SPECIAL_STRING = "Allinus_yejiliayunachaeryeongryujin20190212"; // 特殊字符串

enum class MSG_TYPE : int{
    USER_LOGIN = 0,
    ADMIN_LOGIN
};

struct UserInfo {
    QString username;
    QString realname;
    QString phone;
    int sex = 0;      // 0:保密 1:男 2:女
    QString building;    // 楼栋
    int floor = 0;       // 楼层
    int housenum;        // 门牌
    double balance;      // 余额
};
Q_DECLARE_METATYPE(UserInfo)
struct NoticeInfo{
    QString title;      // 标题
    QString content;    // 正文
    QString sender;     // 发送人
    QString time;       // 时间
    QColor typeColor;   // 类型颜色
};
// 2. 注册这个结构体，让QVariant能识别它
Q_DECLARE_METATYPE(NoticeInfo)
// 注册结构体的列表
Q_DECLARE_METATYPE(QList<NoticeInfo>)

struct ReportInfo {
    QString username;       // 提交人
    QString content;        // 报修/投诉内容
    QJsonArray images; // 图片路径列表
    // 1:报修 2:投诉
    int type = 1;
    // (1:不急 ... 5:很急)
    int level = 1;
    // 0:待处理 1:处理中 2:已完成
    int status = 0;
    QString time; // 提交时间
    QString imagesPath;
};
Q_DECLARE_METATYPE(ReportInfo)

struct BillInfo{
    QString title;
    double spend; // 花费
    QString subInfo;
    QVector<double> dataList = {}; // 每月情况

};
Q_DECLARE_METATYPE(BillInfo)
Q_DECLARE_METATYPE(QList<BillInfo>)

struct ChangeBillsInfo{
    QString username;
    double balance; // 顺便在这里改一下余额

    double waterSpend;
    double electSpend;
    double gasSpend;
    QString parkSpend;
    QString propertySpend;
};
Q_DECLARE_METATYPE(ChangeBillsInfo)

struct PaymentInfo{
    double amount; // 缴费金额
    QString title; // 哪个费用
    QString time; // 缴费时间
};
Q_DECLARE_METATYPE(PaymentInfo)
Q_DECLARE_METATYPE(QList<PaymentInfo>)

struct UserAllInfo{
    QString username;
    QString realname;
    QString phone;
    QString building;
    int floor;
    int housenum;
    double balance;
    double water_spend;
    QString water_info;
    double elect_spend;
    QString elect_info;
    double gas_spend;
    QString gas_info;
    int temperature;
    QString area;
    QString parking_date;
    QString property_date;
};
Q_DECLARE_METATYPE(UserAllInfo)
Q_DECLARE_METATYPE(QList<UserAllInfo>)

struct DeliveryInfo{
    QString username;
    QString content;
    QString imagesPath;
    int type;
    int status;
};
Q_DECLARE_METATYPE(DeliveryInfo)

struct FinalDeliveryInfo{
    QString username;
    QString phone;
    QString address;
    QString content;
    QVector<QByteArray> imgs;
    int type;
    int status;
};
Q_DECLARE_METATYPE(FinalDeliveryInfo)

#endif // GLOBAL_H
