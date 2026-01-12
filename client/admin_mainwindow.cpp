#include "admin_mainwindow.h"
#include "ui_admin_mainwindow.h"

#include <QMessageBox>


Admin_MainWindow::Admin_MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Admin_MainWindow)
{
    ui->setupUi(this);
    m_sock = &Socket::getInstance();

    m_sock->SendGetProgressRequest("");
    m_sock->SendAdminGetAllUserInfoRequest();

    connect(m_sock, &Socket::adminProgressResult, this, &Admin_MainWindow::onProgressResult);
    connect(m_sock, &Socket::allUserInfoResult, this, &Admin_MainWindow::onallUserInfoResult);
    connect(m_sock, &Socket::deliveryInfoResult, this, &Admin_MainWindow::onDeliveryInfoResult);
    connect(m_sock, &Socket::updateDeliveryStatusResult, this, [this](){
        m_sock->SendGetProgressRequest("");
    });

    // 连接聊天服务器，因为要主动向服务器发送
    Socket::getInstance().ConnectChatServer("root", "127.0.0.1", 4389); // 修改

    connect(ui->pending_widget, &QListWidget::itemClicked, this, &Admin_MainWindow::onItemClicked);

    InitUi();
}

Admin_MainWindow::~Admin_MainWindow()
{
    delete ui;
}

void Admin_MainWindow::InitUi()
{
    ui->send_btn->setCursor(Qt::PointingHandCursor);
    ui->level3_rb->setCursor(Qt::PointingHandCursor);
    ui->level2_rb->setCursor(Qt::PointingHandCursor);
    ui->level1_rb->setCursor(Qt::PointingHandCursor);

    ui->sender_edit->setPlaceholderText("在此输入发送方...");
    ui->title_edit->setPlaceholderText("在此输入公告标题...");

    ui->pending_widget->setStyleSheet(
        "QListWidget {"
        "    border: none;"
        "    outline: none;"
        "    padding: 0px;"
        "    margin: 0px;"
        "}"
        "QListWidget::item {"
        "background-color: rgba(33, 33, 33, 50);"
        "border-bottom: 1px solid rgba(0, 0, 0, 100);"
        "border-radius: 0px;"
        "}"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;" /* 轨道透明 */
        "    width: 8px;"              /* 滚动条很细 */
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(0, 0, 0, 80);"  // 半透明白（暗色主题友好）
        "    border-radius: 4px;"
        "    min-height: 14px;"
        "    width: 6px;"
        "    padding: 0px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;" /* 隐藏上下的箭头 */
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;" /* 隐藏剩余空间的背景 */
        "}"
    );

    QStringList header;
    header << "用户名" << "真实姓名" << "手机号" << "账户余额" << "楼栋" << "楼层" << "单元号" << "水用量(m³)" << "水费(元)"
           << "电用量(kWh)" << "电费(元)" << "燃气用量(m³)" << "燃气费(元)" << "室内温度(°C)" << "住房面积(m²)" << "停车年卡截止期" << "物业费截止期";
    //ui->userInfo_Widget->setColumnCount(header.size()); // 获取列数
    ui->userInfo_Widget->setHorizontalHeaderLabels(header);
    ui->userInfo_Widget->resizeColumnsToContents();
    ui->userInfo_Widget->setStyleSheet(
        "QHeaderView::section {"
        "background-color: #f0f0f0;"
        "padding: 4px;"
        "border: 1px solid #d0d0d0;"  /* 边框 */
        "border-left: none;"          /* 去掉左边框防止重叠 */
        "font-weight: bold;"          /* 字体加粗 */
        "}"
    );
}

void Admin_MainWindow::AddTableRow(UserAllInfo info)
{
    // 不显示前面的行号
    //ui->userInfo_Widget->verticalHeader()->setVisible(false);
    int row = ui->userInfo_Widget->rowCount();
    ui->userInfo_Widget->insertRow(row);
    // 创建居中item
    auto createItem = [](const QString& text) {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter); // 默认居中
        return item;
    };
    ui->userInfo_Widget->setItem(row, 0, createItem(info.username));
    ui->userInfo_Widget->setItem(row, 1, createItem(info.realname));
    ui->userInfo_Widget->setItem(row, 2, createItem(info.phone));
    ui->userInfo_Widget->setItem(row, 3, createItem(QString::number(info.balance, 'f', 2)));
    ui->userInfo_Widget->setItem(row, 4, createItem(info.building));
    ui->userInfo_Widget->setItem(row, 5, createItem(QString::number(info.floor)));
    ui->userInfo_Widget->setItem(row, 6, createItem(QString::number(info.housenum)));
    ui->userInfo_Widget->setItem(row, 7, createItem(QString::number(info.water_spend, 'f', 1)));
    ui->userInfo_Widget->setItem(row, 8, createItem(QString::number(info.water_spend * info.water_info.toDouble(), 'f', 2)));
    ui->userInfo_Widget->setItem(row, 9, createItem(QString::number(info.elect_spend, 'f', 1)));
    ui->userInfo_Widget->setItem(row, 10, createItem(QString::number(info.elect_spend * info.elect_info.toDouble(), 'f', 2)));
    ui->userInfo_Widget->setItem(row, 11, createItem(QString::number(info.gas_spend, 'f', 1)));
    ui->userInfo_Widget->setItem(row, 12, createItem(QString::number(info.gas_spend * info.gas_info.toDouble(), 'f', 2)));
    ui->userInfo_Widget->setItem(row, 13, createItem(QString::number(info.temperature, 'f', 0)));
    ui->userInfo_Widget->setItem(row, 14, createItem(info.area));
    ui->userInfo_Widget->setItem(row, 15, createItem(info.parking_date));
    ui->userInfo_Widget->setItem(row, 16, createItem(info.property_date));
}

void Admin_MainWindow::onProgressResult(bool success, QString message, const QList<ReportInfo> &list)
{
    if(success){
        qDebug() << "获取处理进度成功：" << message;
        ui->pending_widget->clearAll();
        for(const ReportInfo& info : list){
            ui->pending_widget->addAdminReportItem(info);
        }
    }
    else{
        qDebug() << "获取处理进度失败：" << message;
    }
}

void Admin_MainWindow::onallUserInfoResult(bool success, QString message, const QList<UserAllInfo> &list)
{
    if(success){
        qDebug() << "获取处理进度成功：" << message;
        ui->userInfo_Widget->setRowCount(0);
        for(const UserAllInfo& info : list){
            AddTableRow(info);
        }
        ui->userInfo_Widget->resizeColumnsToContents();
    }
    else{
        qDebug() << "获取处理进度失败：" << message;
    }
}

void Admin_MainWindow::onDeliveryInfoResult(bool success, QString message, QString phone, QString address, const QVector<QByteArray> &imgsBytes)
{
    if(success){
        qDebug() << "取报修投诉信息成功：" << message;

        m_rptDlg = new AdminReportDialog(this);
        m_rptDlg->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
        m_rptDlg->setWindowModality(Qt::ApplicationModal);
        // 把数据传入dialog
        FinalDeliveryInfo info;
        info.address = address;
        info.phone = phone;
        info.imgs = imgsBytes;
        info.content = m_content;
        info.type = m_type;
        info.status = m_status;
        info.username = m_username;
        m_rptDlg->SetDeliveryInfo(info);

        connect(m_rptDlg, &AdminReportDialog::updateDelivery, this, &Admin_MainWindow::onUpdateDelivery);

        m_rptDlg->show();
    }
    else{
        qDebug() << "获取报修投诉信息失败：" << message;
    }
}

void Admin_MainWindow::onUpdateDelivery(int status)
{
    qDebug() << m_username;
    qDebug() << m_content;
    m_sock->SendUpdateDeliveryStatusRequest(m_username, m_content, status);
}

void Admin_MainWindow::on_send_btn_clicked()
{
    NoticeInfo info;
    info.title = ui->title_edit->text();
    info.content = ui->content_edit->toPlainText();
    info.sender = ui->sender_edit->text();
    info.time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    if(ui->level3_rb->isChecked()) info.typeColor = QColor("#FF4D4F");
    else if(ui->level2_rb->isChecked()) info.typeColor = QColor("#FAAD14");
    else info.typeColor = QColor("#52C41A");
    if(info.title == "" || info.content == "" || info.sender == ""){
        QMessageBox::information(this, "提示", "请将公告内容补充完整");
        return;
    }
    else {
        m_sock->SendPushNoticeRequest(info);
        QMessageBox::information(this, "提示", "公告发布成功");
        ui->title_edit->clear();
        ui->content_edit->clear();
        ui->sender_edit->clear();
        ui->level3_rb->setAutoExclusive(false);
        ui->level2_rb->setAutoExclusive(false);
        ui->level1_rb->setAutoExclusive(false);
        ui->level3_rb->setChecked(false);
        ui->level2_rb->setChecked(false);
        ui->level1_rb->setChecked(false);
    }
    ui->level3_rb->setAutoExclusive(true);
    ui->level2_rb->setAutoExclusive(true);
    ui->level1_rb->setAutoExclusive(true);
}

void Admin_MainWindow::onItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    // 取出存储的DeliveryInfo
    QVariant dataVar = item->data(Qt::UserRole);
    if (dataVar.canConvert<DeliveryInfo>()) {
        DeliveryInfo info = dataVar.value<DeliveryInfo>();
        m_content = info.content;
        m_type = info.type;
        m_status = info.status;
        m_username = info.username;
        m_sock->SendAdminDeliveryInfoRequest(info.username, info.imagesPath);
    }
}

