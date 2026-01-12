#ifndef IMAGESLOT_H
#define IMAGESLOT_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class ImageSlot : public QWidget
{
    Q_OBJECT
public:
    explicit ImageSlot(QWidget *parent = nullptr);

public:
    void SetXY(int x, int y);
    void SetPath(const QString& path);
    QString GetPath() const{ return m_path; }
signals:
    void clickUpload(); // 点 加
    void clickDelete(); // 点 叉
protected:
    void mousePressEvent(QMouseEvent *event) override;
private:
    QLabel* m_imageLb;
    QPushButton* m_delBtn;
    QString m_path;
    int m_x;
    int m_y;
};

#endif // IMAGESLOT_H
