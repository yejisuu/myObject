#ifndef ADMINREPORTDIALOG_H
#define ADMINREPORTDIALOG_H

#include <QDialog>
#include "global.h"
#include <QLabel>


namespace Ui {
class AdminReportDialog;
}

class AdminReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdminReportDialog(QWidget *parent = nullptr);
    ~AdminReportDialog();
public:
    void SetDeliveryInfo(const FinalDeliveryInfo& info);
    void InitUi();
signals:
    void updateDelivery(int status);
private slots:
    void on_deal_btn_clicked();

    void on_Processed_btn_clicked();

private:
    Ui::AdminReportDialog *ui;
    QVector<QLabel*> m_labelArray;
};

#endif // ADMINREPORTDIALOG_H
