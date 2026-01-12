#include "historylistwidget.h"
#include "ui_historylistwidget.h" // 这一行就是引用那个自动生成的UI头文件
#include "reportitemwidget.h"     // 引用单行条目类
#include <QScrollBar>

HistoryListWidget::HistoryListWidget(QWidget *parent) :
    QListWidget(parent),
    ui(new Ui::HistoryListWidget)
{
    ui->setupUi(this);

    // 设置按像素平滑滚动
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->verticalScrollBar()->setSingleStep(5); // 5像素

}

HistoryListWidget::~HistoryListWidget()
{
    delete ui;
}

void HistoryListWidget::SetUi()
{
    //this->setStyleSheet("background-color: transparent;");
    // 设置背景透明，去掉边框
    this->setStyleSheet(
        // 1. 列表整体容器
        "QListWidget {"
        "    /* 背景：深色半透明 (模拟暗色玻璃) */"
        "    background-color: rgba(0, 0, 0, 40);"
        ""
        "    /* 边框：极细的白线，模拟玻璃边缘反光 */"
        "    border: 1px solid rgba(255, 255, 255, 60);"
        "    border-radius: 12px;"
        ""
        "    /* 内边距：让里面的内容不要贴着边框 */"
        "    padding: 5px;"
        "    outline: none; /* 去掉选中时的虚线框 */"
        "}"
        ""
        // 2. 列表项
        "QListWidget::item {"
        "    /* 给每一行加个透明边框，防止文字贴边 */"
        "    border-bottom: 1px solid rgba(255, 255, 255, 10);"
        "    border-radius: 8px;"
        "}"
        ""
        // 3. 鼠标悬停在某一行时
        "QListWidget::item:hover {"
        "    /* 背景变亮一点点 */"
        "    background-color: rgba(255, 255, 255, 20);"
        "    border: 1px solid rgba(255, 255, 255, 60);"
        "}"
        ""
        // 4. 选中某一行时
        "QListWidget::item:selected {"
        "    background-color: rgba(255, 255, 255, 30);"
        "    border: 1px solid rgba(255, 255, 255, 100);"
        "}"
        ""

        // 滚动条美化
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;" /* 轨道透明 */
        "    width: 11px;"              /* 滚动条很细 */
        "    margin: 0px;"
        "    padding-left: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(255, 255, 255, 40);" /* 滑块半透明白 */
        "    border-radius: 3px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(255, 255, 255, 80);" /* 鼠标放上去变亮 */
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;" /* 隐藏上下的箭头 */
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;" /* 隐藏剩余空间的背景 */
        "}"
    );

}

void HistoryListWidget::addReportItem(const ReportInfo& info, bool isNew)
{
    // 创建列表项载体
    QListWidgetItem* item = new QListWidgetItem();

    // 创建我们自定义的内容 Widget
    ReportItemWidget* widget = new ReportItemWidget();
    widget->setData(info);

    // 必须设置item的大小，否则列表会挤在一起看不见
    item->setSizeHint(QSize(0, 40));

    if(isNew){
        this->insertItem(0,item);
        this->scrollToItem(item);
    }
    else{
        this->addItem(item);
    }
    // 把Widget塞进Item里
    this->setItemWidget(item, widget);
}

void HistoryListWidget::addAdminReportItem(const ReportInfo &info)
{
    // 创建列表项载体
    QListWidgetItem* item = new QListWidgetItem();
    // 创建我们自定义的内容 Widget
    ReportItemWidget* widget = new ReportItemWidget();
    widget->setAdminData(info);

    // 必须设置item的大小，否则列表会挤在一起看不见
    item->setSizeHint(QSize(0, 40));

    // 把数据存入item
    DeliveryInfo Dinfo;
    Dinfo.content = info.content;
    Dinfo.imagesPath = info.imagesPath;
    Dinfo.type = info.type;
    Dinfo.username = info.username;
    Dinfo.status = info.status;
    item->setData(Qt::UserRole, QVariant::fromValue(Dinfo));

    connect(widget, &ReportItemWidget::clicked, this, [this, item]() {
        emit itemClicked(item);
    });

    this->addItem(item);
    // 把Widget塞进Item里
    this->setItemWidget(item, widget);
}

void HistoryListWidget::addPayOptItem(const QString &title, const QString &useInfo, double amount)
{
    QListWidgetItem* item = new QListWidgetItem();
    ReportItemWidget* widget = new ReportItemWidget();
    widget->setPayData(title, useInfo, amount);
    item->setSizeHint(QSize(0, 40));
    this->addItem(item);
    this->setItemWidget(item, widget);
}

void HistoryListWidget::addPaymentItem(const PaymentInfo& info, bool isNew)
{
    // 创建列表项载体
    QListWidgetItem* item = new QListWidgetItem();

    // 创建我们自定义的内容 Widget
    ReportItemWidget* widget = new ReportItemWidget();
    widget->setPaymentData(info);

    // 必须设置item的大小，否则列表会挤在一起看不见
    item->setSizeHint(QSize(0, 40));

    if(isNew){
        this->insertItem(0,item);
        this->scrollToItem(item);
    }
    else{
        this->addItem(item);
    }
    // 把Widget塞进Item里
    this->setItemWidget(item, widget);
}

void HistoryListWidget::delPayOptItem(const QString &title)
{
    for (int i = this->count() - 1; i >= 0; --i) {
        QListWidgetItem* item = this->item(i);
        // 获取Item对应的自定义 Widget
        QWidget* widget = this->itemWidget(item);
        // 将Widget强转为ReportItemWidget
        ReportItemWidget* reportItem = qobject_cast<ReportItemWidget*>(widget);

        if (reportItem) {
            if (reportItem->getTitle() == title) {
                // 移除并删除
                this->takeItem(i);
                delete item; // 这一步会自动把 reportItem 也删掉
                //return;
            }
        }
        m_itemCount = this->count();
    }
}

QList<PaymentInfo> HistoryListWidget::GetPayInfoList()
{
    QList<PaymentInfo> list;
    for (int i = this->count() - 1; i >= 0; --i) {
        PaymentInfo info;
        QListWidgetItem* item = this->item(i);
        // 获取Item对应的自定义 Widget
        QWidget* widget = this->itemWidget(item);
        // 将Widget强转为ReportItemWidget
        ReportItemWidget* reportItem = qobject_cast<ReportItemWidget*>(widget);

        if (reportItem) {
            info.title = reportItem->getTitle();
            info.amount = reportItem->getAmount();
            info.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        }
        list.push_back(info);
    }
    return list;
}

void HistoryListWidget::clearAll()
{
    this->clear();
}
