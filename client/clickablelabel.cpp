#include "clickablelabel.h"


clickablelabel::clickablelabel(QWidget *parent)
    : QLabel(parent)
{
    // 默认设置鼠标为小手形状
    this->setCursor(Qt::PointingHandCursor);
}

void clickablelabel::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果是鼠标左键点击
    if (event->button() == Qt::LeftButton) {
        emit clicked(); // 发射信号
    }
    else if (event->button() == Qt::RightButton) {
        emit sendClicked(); // 发射信号
    }
    // 调用父类处理，保证其他功能正常
    QLabel::mouseReleaseEvent(event);
}
