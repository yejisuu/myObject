#ifndef REPORTITEMWIDGET_H
#define REPORTITEMWIDGET_H

#include <QWidget>
#include "global.h"
#include <QMouseEvent>

namespace Ui {
class ReportItemWidget;
}

class ReportItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReportItemWidget(QWidget *parent = nullptr);
    ~ReportItemWidget();
public:
    // 设置数据
    void setData(const ReportInfo& info);
    // 设置管理员界面数据
    void setAdminData(const ReportInfo& info);
    // 设置缴费数据
    void setPayData(const QString& title, const QString& useInfo, double amount);
    // 设置缴费记录数据
    void setPaymentData(const PaymentInfo& info);
    // 获取头
    QString getTitle();
    double getAmount();

protected:
    void mousePressEvent(QMouseEvent* event) override;  // 重写鼠标按下事件
signals:
    void clicked();
private:
    Ui::ReportItemWidget *ui;
    double m_amount;

};

#endif // REPORTITEMWIDGET_H
