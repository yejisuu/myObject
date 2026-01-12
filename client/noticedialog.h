#ifndef NOTICEDIALOG_H
#define NOTICEDIALOG_H

#include <QDialog>
#include <QMouseEvent>

namespace Ui {
class NoticeDialog;
}

class NoticeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NoticeDialog(QWidget *parent = nullptr);
    ~NoticeDialog();
public:
    void SetLogContent(const QString &title, const QString &content, const QString &sender, const QString &time, const QColor& color);
protected:
    // 重写这两个事件来实现窗口拖动
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    Ui::NoticeDialog *ui;
    QPoint m_dragPosition;
};

#endif // NOTICEDIALOG_H
