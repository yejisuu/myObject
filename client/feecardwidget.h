#ifndef FEECARDWIDGET_H
#define FEECARDWIDGET_H

#include <QWidget>

namespace Ui {
class FeeCardWidget;
}

class FeeCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeeCardWidget(QWidget *parent = nullptr);
    ~FeeCardWidget();
public:
    void SetFeeInfo();
    // 更新余额
    void UpdateBalance();
private:
    Ui::FeeCardWidget *ui;

    int m_id;
    QString m_name;
    QString m_uint;
};

#endif // FEECARDWIDGET_H
