#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

class clickablelabel: public QLabel
{
    Q_OBJECT
public:
    explicit clickablelabel(QWidget *parent = nullptr);
signals:
    void clicked();
    void sendClicked();
protected:
    // 重写鼠标点击松开事件
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // CLICKABLELABEL_H
