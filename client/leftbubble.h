#ifndef LEFTBUBBLE_H
#define LEFTBUBBLE_H

#include <QWidget>
#include <QMenu>

namespace Ui {
class LeftBubble;
}

class LeftBubble : public QWidget
{
    Q_OBJECT

public:
    explicit LeftBubble(QWidget *parent = nullptr);
    ~LeftBubble();
public:
    // 设置内容的接口
    void SetText(const QString& text);
    void SetTime(const QString& time);
    void SetSender(const QString& sender);
    void SetAvatar(const QString& path);
    void SetContentColor(QColor color);
signals:
    void privateSend(QString username);
public slots:
    void onClicked();
private:
    Ui::LeftBubble *ui;
    QString m_username;
};

#endif // LEFTBUBBLE_H
