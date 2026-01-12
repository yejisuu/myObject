#ifndef BUBBLELABEL_H
#define BUBBLELABEL_H

#include <QLabel>
#include <QPainter>
#include <QPainterPath>

class BubbleLabel : public QLabel
{
    Q_OBJECT
public:
    explicit BubbleLabel(QWidget *parent = nullptr);

public:
    enum BubbleType { Left, Right };

    void setType(BubbleType type);
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    BubbleType m_type;
};

#endif // BUBBLELABEL_H
