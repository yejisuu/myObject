#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
#include "imageslot.h"
#include "resultdialog.h"

namespace Ui {
class ReportWidget;
}

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(QWidget *parent = nullptr);
    ~ReportWidget();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
private:
    void InitUi();
    // 初始化星星
    void InitStars();
    void RefreshSlots();
    // 刷新星星状态
    void UpdateStars(int level);
    // 检查信息写全了没
    bool CheckMessage();
    // 提交完请求，清空内容
    void InitRequest();
signals:
    // 投递投诉或报修
    void toDelivery(int type, QString content, int level, QStringList imagePaths, int status, QString time);
private slots:
    void onHandleUpload(); // 处理上传
    void onHandleDelete(int index); // 处理删除，刷新槽位
    void on_toReport_btn_clicked();
    void on_toComplaint_btn_clicked();

private:
    Ui::ReportWidget *ui;
    // 五个图片槽位
    QList<ImageSlot*> m_slots;
    // 五个星星
    QList<QPushButton*> m_btns;
    int m_currentLevel;
    // 存图片路径
    QStringList m_imagePaths;
    ResultDialog* m_resDlg;
    int m_level; // 记录选中的等级
};

#endif // REPORTWIDGET_H
