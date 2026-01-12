#ifndef RIGHTBUBBLE_H
#define RIGHTBUBBLE_H

#include <QWidget>

namespace Ui {
class RightBubble;
}

class RightBubble : public QWidget
{
    Q_OBJECT

public:
    explicit RightBubble(QWidget *parent = nullptr);
    ~RightBubble();
public:
    // 设置内容的接口
    void SetText(const QString& text);
    void SetAvatar(const QString& path);
    void SetContentColor(QColor color);
private:
    Ui::RightBubble *ui;
};

#endif // RIGHTBUBBLE_H
