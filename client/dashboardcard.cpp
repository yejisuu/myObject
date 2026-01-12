#include "DashboardCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QMouseEvent>
#include <QDebug>

DashboardCard::DashboardCard(QWidget *parent) : QWidget(parent)
{
    // 允许QWidget响应Designer设置的 QSS 背景
    setAttribute(Qt::WA_StyledBackground, true);
    // 鼠标追踪，用于悬停效果
    setMouseTracking(true);
    //this->setCursor(Qt::PointingHandCursor);
    m_style = Style_Chart;
    m_percent = 0.0;
    m_themeColor = QColor(0, 122, 204);
    m_isRing = false;

    // 顶部标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("color: #ffffff; font-size: 20pt; font-weight: bold; background: transparent;");
    m_titleLabel->move(10, 5);

    // 底部信息行
    m_subLabel = new QLabel(this);
    m_subLabel->setStyleSheet("color: #ffffff; font-size: 12pt; background: transparent;");
    m_subLabel->move(5, 110);

    m_spendLabel = new QLabel(this);
    m_spendLabel->setFixedSize(135, 40);
    m_spendLabel->setStyleSheet("font-size: 20pt; font-weight: bold; background: transparent;");
    m_spendLabel->setAlignment(Qt::AlignRight);
    m_spendLabel->move(125, 100);

    m_semLabel = new QLabel(this);
    m_semLabel->setFixedSize(110, 40);
}

void DashboardCard::SetData(const QString &title, const QString &spend, const QString &subInfo,
                            CardStyle style, const QVector<double> &dataList, double percent, QColor themeColor)
{
    m_titleLabel->setText(title);
    m_spendLabel->setText(spend);
    m_subLabel->setText(subInfo);
    m_style = style;
    m_dataList = dataList;
    m_percent = percent;
    m_themeColor = themeColor;

    // 动态设置右下角数值的颜色
    QString valueStyle = QString("color: %1; font-size: 20pt; font-weight: bold; background: transparent;").arg(themeColor.name());
    m_spendLabel->setStyleSheet(valueStyle);

    if(style == Style_Ring){
        m_isRing = true;
        m_spendLabel->move(125, 5);
        m_spendLabel->setStyleSheet("color: #ffffff; font-size: 16pt;");
        m_subLabel->move(45, 110);
        m_subLabel->setStyleSheet("color: #ffffff; font-size: 14pt; background: transparent;");   
    }
    else if(style == Style_Bar){
        if(m_percent <= 0) m_themeColor = QColor(0, 51, 160);
        else if(m_percent > 0 && m_percent <= 18) m_themeColor = QColor(50, 153, 204);
        else if(m_percent > 18 && m_percent <= 30) m_themeColor = QColor(255, 153, 51);
        else m_themeColor = QColor(204, 0, 0);

        QString tem = QString::number(m_percent) + "°C";
        QString style = QString("color: %1; font-size: 35pt; font-weight: bold; background: transparent;").arg(m_themeColor.name());
        m_semLabel->setStyleSheet(style);
        m_semLabel->setText(tem);
        m_semLabel->move(80, 45);
        m_semLabel->show();
    }
    update(); // 触发重绘
}

BillInfo DashboardCard::GetBillInfo()
{
    BillInfo info;
    info.title = m_titleLabel->text();
    info.spend = m_spend;
    if(m_content != "") info.subInfo = m_content;
    else info.subInfo = "有效期" + m_time.toString(("yyyy/MM/dd"));

    return info;
}

void DashboardCard::SetModifiableData(double spend,  QString content, QDate* time)
{
    m_spend = spend;
    m_content = content;
    m_time = *time;
}

QString DashboardCard::UpdateData(double spend)
{
    if(spend == -1){
        m_time = m_time.addYears(1);
        QDate currentDate = QDate::currentDate();
        int daysDiff = currentDate.daysTo(m_time);
        int yearsDiff = m_time.year() - currentDate.year();
        qDebug() << yearsDiff;
        m_percent = daysDiff / (365.00 * (yearsDiff + 1));

        if(m_percent > 0 && m_percent <= 0.25) m_themeColor = QColor(255, 0, 0);
        else if(m_percent > 0.25 && m_percent <= 0.5) m_themeColor = QColor(255, 165, 0);
        else if(m_percent > 0.5 && m_percent <= 0.75) m_themeColor = QColor(173, 255, 47);
        else m_themeColor = QColor(0, 255, 0);
        update();
        QString time = m_time.toString("yyyy-MM-dd");
        m_subLabel->setText("有效期至:" + time);
        return time;
    }
    else{
        m_spendLabel->setText("¥" + QString::number(spend, 'f', 2));
        m_spend = 0;
    }
    return "";
}

void DashboardCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    // 计算中间绘图区域 (避开文字)
    int topMargin = 35;  // 避开标题
    int bottomMargin = 40; // 避开底部文字
    QRect contentRect(15, topMargin, rect().width() - 30, rect().height() - topMargin - bottomMargin);

    // 根据模式绘制图形
    switch (m_style) {
    case Style_Chart:
        DrawChart(painter, contentRect);
        break;
    case Style_Ring:
        DrawRing(painter, contentRect);
        break;
    case Style_Bar:
        break;
    }
}

// 模式1：曲线面积图
void DashboardCard::DrawChart(QPainter &painter, const QRect &rect)
{
    if (m_dataList.isEmpty()) return;

    double maxVal = -999999; // 初始化为极小值
    double minVal = 999999;  // 初始化为极大值

    for (double v : m_dataList) {
        if (v > maxVal) maxVal = v;
        if (v < minVal) minVal = v;
    }

    // --- 2. 计算动态范围 (Range) ---
    double range = maxVal - minVal;

    // 如果所有数据都一样 (range == 0)，或者数据全是0，做个特殊处理防止除以0
    if (range == 0) {
        range = (maxVal == 0) ? 1.0 : maxVal * 0.1;
    }

    // --- 3. 增加上下留白 (Padding) ---
    // 如果不加留白，最低点会贴着底边，最高点会贴着顶边，不好看。
    // 我们在上下各增加 20% 的空间
    double padding = range * 0.2;

    // 修正后的绘图上下限
    double plotMin = minVal - padding;
    double plotMax = maxVal + padding;
    double plotRange = plotMax - plotMin;

    // 防止 plotRange 极小导致显示异常（比如数据就是 140.001 和 140.002）
    if (plotRange == 0) plotRange = 1.0;

    QPainterPath path;
    double stepX = (double)rect.width() / (m_dataList.size() - 1);

    // --- 4. 计算第一个点 (使用新的公式) ---
    // 公式：(当前值 - 绘图下限) / 绘图总范围
    double firstRatio = (m_dataList[0] - plotMin) / plotRange;
    double firstY = rect.bottom() - firstRatio * rect.height();

    path.moveTo(rect.left(), firstY);

    for (int i = 1; i < m_dataList.size(); ++i) {
        double x = rect.left() + i * stepX;

        // 使用新的公式计算 Y
        double ratio = (m_dataList[i] - plotMin) / plotRange;
        double y = rect.bottom() - ratio * rect.height();

        path.lineTo(x, y);
    }

    // 绘制填充
    QPainterPath fillPath = path;
    fillPath.lineTo(rect.right(), rect.bottom());
    fillPath.lineTo(rect.left(), rect.bottom());
    fillPath.closeSubpath();

    QLinearGradient gradient(0, rect.top(), 0, rect.bottom());
    gradient.setColorAt(0, QColor(m_themeColor.red(), m_themeColor.green(), m_themeColor.blue(), 100)); // 半透明
    gradient.setColorAt(1, QColor(m_themeColor.red(), m_themeColor.green(), m_themeColor.blue(), 0));   // 全透明
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawPath(fillPath);

    // 绘制线条
    QPen pen(m_themeColor, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

// 模式2：环形进度 (适合展示百分比)
void DashboardCard::DrawRing(QPainter &painter, const QRect &rect)
{

    int size = qMin(rect.width(), rect.height());
    // 居中
    QRect ringRect(rect.center().x() - size/2, rect.center().y() - size/2, size, size);
    ringRect.adjust(5, 5, -5, -5); // 稍微缩一点

    int startAngle = 90 * 16; // 从12点钟开始
    int spanAngle = -m_percent * 360 * 16; // 逆时针画

    // 画底色槽
    QPen bgPen(QColor(230, 230, 230), 6, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(bgPen);
    painter.drawEllipse(ringRect);

    // 画进度
    QPen progPen(m_themeColor, 6, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progPen);
    painter.drawArc(ringRect, startAngle, spanAngle);

    // 中间显示百分比小字 (可选)
    painter.setPen(m_themeColor);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(ringRect, Qt::AlignCenter, QString::number(int(m_percent * 100)) + "%");
}


void DashboardCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(); // 发送点击信号
    }
    QWidget::mousePressEvent(event);
}


