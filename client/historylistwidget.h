#ifndef HISTORYLISTWIDGET_H
#define HISTORYLISTWIDGET_H

#include <QWidget>
#include "global.h"
#include <QListWidgetItem>

namespace Ui {
class HistoryListWidget;
}

class HistoryListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit HistoryListWidget(QWidget *parent = nullptr);
    ~HistoryListWidget();
public:
    void SetUi();
    // 添加一条投诉/报修记录
    void addReportItem(const ReportInfo& info, bool isNew = false);
    // 添加管理员显示的投诉/报修记录
    void addAdminReportItem(const ReportInfo& info);
    // 添加一个支付选项
    void addPayOptItem(const QString& title, const QString& useInfo, double amount);
    // 添加一个支付账单记录
    void addPaymentItem(const PaymentInfo& info, bool isNew = false);
    // 删除一个账单记录
    void delPayOptItem(const QString& title);
    // 获取每个支付的item的信息
    QList<PaymentInfo> GetPayInfoList();
    // 清空所有记录
    void clearAll();
    // 获取item数量
    int GetItemCount(){ return m_itemCount; }

private:
    Ui::HistoryListWidget *ui;
    int m_itemCount;
};

#endif // HISTORYLISTWIDGET_H
