#include "bubblelabel.h"

BubbleLabel::BubbleLabel(QWidget *parent)
    : QLabel(parent)
{
    m_type = Left;
    // 开启自动换行
    this->setWordWrap(true);
    // 设置文字内边距，防止文字盖住小三角
    // 初始值随便设，后面 setType 会覆盖
    this->setContentsMargins(15, 10, 10, 10);
}

void BubbleLabel::setType(BubbleType type)
{
    m_type = type;
    if (m_type == Left) {
        // 别人发的：三角在左边，文字左边距要大一点
        this->setContentsMargins(5 + 6, 6, 6, 6); // 6是三角宽度
        this->setStyleSheet("color: white;"); // 字体黑色
    } else {
        // 我发的：三角在右边，文字右边距要大一点
        this->setContentsMargins(6, 6, 5 + 6, 6);
        this->setStyleSheet("color: white;");
    }
    update(); // 触发重绘
}

void BubbleLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿，不然线条有锯齿

    // 设置颜色
    QColor bgColor;
    if (m_type == Left) {
        bgColor = QColor(255, 255, 255, 100); // 白底
    } else {
        bgColor = QColor(149, 236, 105, 100); // 微信绿
    }

    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen); // 不要边框线

    // 计算几何参数
    int w = width();
    int h = height();
    int triangleWidth = 6;  // 三角的宽
    int triangleHeight = 10; // 三角的高
    int radius = 4;          // 圆角大小
    int triangleY = 14;      // 三角尖尖距离顶部的距离

    QPainterPath path;

    // 开始画路径
    if (m_type == Left) {
        // --- 左侧气泡 ---
        // 从三角尖尖开始画
        path.moveTo(0, triangleY);
        path.lineTo(triangleWidth, triangleY - triangleHeight/2);

        // 上边线 + 右上圆角
        path.lineTo(triangleWidth, radius);
        path.arcTo(triangleWidth, 0, radius*2, radius*2, 180, -90);
        path.lineTo(w - radius, 0);
        path.arcTo(w - radius*2, 0, radius*2, radius*2, 90, -90);

        // 右边线 + 右下圆角
        path.lineTo(w, h - radius);
        path.arcTo(w - radius*2, h - radius*2, radius*2, radius*2, 0, -90);

        // 下边线 + 左下圆角
        path.lineTo(triangleWidth + radius, h);
        path.arcTo(triangleWidth, h - radius*2, radius*2, radius*2, 270, -90);

        // 回到三角底部
        path.lineTo(triangleWidth, triangleY + triangleHeight/2);
        path.closeSubpath();
    }
    else {
        // --- 右侧气泡 ---
        // 从左上圆角开始
        path.moveTo(radius, 0);
        path.lineTo(w - triangleWidth - radius, 0);
        path.arcTo(w - triangleWidth - radius*2, 0, radius*2, radius*2, 90, -90);

        // 三角上方
        path.lineTo(w - triangleWidth, triangleY - triangleHeight/2);
        // 三角尖尖
        path.lineTo(w, triangleY);
        // 三角下方
        path.lineTo(w - triangleWidth, triangleY + triangleHeight/2);

        // 右下圆角
        path.lineTo(w - triangleWidth, h - radius);
        path.arcTo(w - triangleWidth - radius*2, h - radius*2, radius*2, radius*2, 0, -90);

        // 左下圆角
        path.lineTo(radius, h);
        path.arcTo(0, h - radius*2, radius*2, radius*2, 270, -90);

        // 左上圆角
        path.lineTo(0, radius);
        path.arcTo(0, 0, radius*2, radius*2, 180, -90);

        path.closeSubpath();
    }

    // 绘制背景
    painter.drawPath(path);

    QLabel::paintEvent(event);
}
