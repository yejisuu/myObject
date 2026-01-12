#ifndef DASHBOARDCARD_H
#define DASHBOARDCARD_H

#include <QWidget>
#include <QLabel>
#include <QVector>
#include "global.h"

class DashboardCard : public QWidget
{
    Q_OBJECT

public:
    // 定义可视化风格枚举
    enum CardStyle {
        Style_Chart,    // 1. 面积曲线图 (默认)
        Style_Ring,     // 2. 环形百分比图
        Style_Bar       // 3. 条形进度条
    };

    explicit DashboardCard(QWidget *parent = nullptr);
    // 程序启动后，获取内容
    void SetData(const QString& title,
                 const QString& spend,
                 const QString& subInfo,
                 CardStyle style = Style_Chart,
                 const QVector<double>& dataList = {},
                 double percent = 0.0,
                 QColor themeColor = QColor(255, 255, 255));

    BillInfo GetBillInfo();
    void SetModifiableData(double spend, QString content, QDate* time = new QDate());
    QString UpdateData(double spend = 0);
signals:
    void clicked(); // 点击信号

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void DrawChart(QPainter& painter, const QRect& rect); // 画曲线
    void DrawRing(QPainter& painter, const QRect& rect);  // 画环形

private:
    // UI 控件
    QLabel* m_titleLabel;
    QLabel* m_subLabel;
    QLabel* m_spendLabel;
    QLabel* m_semLabel;

    // 数据存储
    CardStyle m_style;
    QVector<double> m_dataList;
    double m_percent;
    QColor m_themeColor;
    double m_spend;
    QDate m_time;
    QString m_content;

    bool m_isRing;
};

#endif // DASHBOARDCARD_H
