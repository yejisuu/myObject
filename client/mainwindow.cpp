#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QFileDialog>
#include <QCoreApplication> // 用于获取程序路径
#include <QFileInfo>        // 用于获取文件后缀名
#include <QDir>             // 用于操作目录
#include <QFile>            // 用于复制文件
#include <QBuffer>
#include <QPainterPath>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPen>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTextEdit>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsOpacityEffect>
#include <QDate>
#include "billsdialog.h"
#include "leftbubble.h"
#include "rightbubble.h"

MainWindow::MainWindow(UserInfo userinfo, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_info(userinfo)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    // 背景透明
    this->setAttribute(Qt::WA_TranslucentBackground);
    // 自动delete
    this->setAttribute(Qt::WA_DeleteOnClose);

    m_sock = &Socket::getInstance();

    m_billDlg = new BillsDialog(this);

    m_isPrivate = false;

    InitUi();
    SetCursorStyle();
    InitPersonalData();

    // 创建按钮组
    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->addButton(ui->home_btn, 0);
    m_btnGroup->addButton(ui->pay_btn, 1);
    m_btnGroup->addButton(ui->notes_btn, 2);
    m_btnGroup->addButton(ui->report_btn, 3);
    m_btnGroup->addButton(ui->personal_btn, 4);
    m_btnGroup->setExclusive(true); // 设置互斥
    connect(m_btnGroup, &QButtonGroup::idClicked, this, [=](int id){
        // 跳转到对应的界面
        ui->stackedWidget->setCurrentIndex(id);
    });

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->close_btn, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->mini_btn, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(ui->rpt_widget, &ReportWidget::toDelivery, this, &MainWindow::onDelivery);

    connect(m_sock, &Socket::avatarResult, this, &MainWindow::onUpdateAvatar);
    connect(m_sock, &Socket::noticeResult, this, &MainWindow::onUpdateNotice);
    connect(m_sock, &Socket::infoResult, this, &MainWindow::onUpdateInfo);
    connect(m_sock, &Socket::progressResult, this, &MainWindow::onUpdateProgress);
    connect(m_sock, &Socket::billsResult, this, &MainWindow::onUpdateBills);
    connect(m_sock, &Socket::paymentResult, this, &MainWindow::onUpdatePayment);

    connect(ui->notice_list, &QListWidget::itemClicked, this, [=](QListWidgetItem *item){
        QVariant variant = item->data(Qt::UserRole);

        if(variant.canConvert<NoticeInfo>()){
            NoticeInfo info = variant.value<NoticeInfo>();
            // 触发弹窗
            ShowContentDialog(info);
        }
    });
    Socket::getInstance().ConnectChatServer(m_username, "127.0.0.1", 4389); // 修改

    connect(&Socket::getInstance(), &Socket::chatMessageReceived, this, [=](QString from, QString text, QString time, bool isPrivate){
        AddChatMessage(from, text, time, isPrivate);
    });

    DownloadAvatarToServer(); // 加载头像
    m_sock->SendGetNoticeRequest(m_username); // 获取个人信息
    m_sock->SendGetPaymentRecordRequest(m_username); // 获取缴费账单
    GetProcessProgress(); // 获取处理进度
    GetBillsInfo(); // 获取账单信息

    // 主界面最上面的半透明图片
    ui->title_lb->setStyleSheet("border-image: url(:/image/image/title.jpg); border-radius: 10px; border: 1px solid rgba(255, 255, 255, 20);");
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(ui->title_lb);
    opacityEffect->setOpacity(0.3);
    ui->title_lb->setGraphicsEffect(opacityEffect);

    ensureWindowOnScreen();

    connect(qApp, &QGuiApplication::screenAdded, this, &MainWindow::ensureWindowOnScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MainWindow::ensureWindowOnScreen);

    // 处理支付成功
    connect(m_billDlg, &BillsDialog::payAllSuccess, this, [=](double spend, QList<PaymentInfo> list){
        ChangeBillsInfo binfo;
        binfo.username = m_username;
        binfo.balance = m_balance;
        binfo.waterSpend = m_waterSpend;
        binfo.electSpend = m_electSpend;
        binfo.gasSpend = m_gasSpend;
        binfo.parkSpend = m_parkDate;
        binfo.propertySpend = m_propertyDate;
        for(PaymentInfo& info : list){
            if(info.title == "水费"){
                ui->water_widget->UpdateData(0);
                binfo.waterSpend = 0;
            }
            else if(info.title == "电费"){
                ui->elect_widget->UpdateData(0);
                binfo.electSpend = 0;
            }
            else if(info.title == "燃气"){
                ui->gas_widget->UpdateData(0);
                binfo.gasSpend = 0;
            }
            else if(info.title == "停车年卡"){
                binfo.parkSpend = ui->parking_widget->UpdateData(-1);
            }
            else if(info.title == "物业费"){
                binfo.propertySpend = ui->property_widget->UpdateData(-1);
            }
            ui->payment_widget->addPaymentItem(info, true);
        }
        // 更新每个账单的数据
        m_sock->SendSetBillsInfoRequest(binfo);
        // 发送缴费记录
        m_sock->SendSetPaymentRecordRequest(m_username, list);

        m_balance -= spend;
        ui->balance_lb->setText(QString::number(m_balance, 'f', 2));

        ui->waterPay_btn->setChecked(false);
        ui->electPay_btn->setChecked(false);
        ui->gasPay_btn->setChecked(false);
        ui->parkPay_btn->setChecked(false);
        ui->propertyPay_btn->setChecked(false);
        ui->waterPay_btn->setText("选择缴费");
        ui->electPay_btn->setText("选择缴费");
        ui->gasPay_btn->setText("选择缴费");
        ui->parkPay_btn->setText("选择缴费");
        ui->propertyPay_btn->setText("选择缴费");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ensureWindowOnScreen()
{
    // 获取当前窗口
    QRect windowRect = frameGeometry();

    // 找到窗口中心所在的屏幕
    QPoint windowCenter = windowRect.center();
    QScreen *screen = QGuiApplication::screenAt(windowCenter);

    // 如果中心不在任何屏幕上（常见于拔掉外接屏后），fallback 到主屏
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    // 获取该屏幕的可用区域（不包括任务栏）
    QRect available = screen->availableGeometry();

    // 如果窗口完全或部分跑出屏幕，矫正位置
    if (!available.intersects(windowRect)) {
        // 完全不在屏幕内 → 移到屏幕中央
        move(available.center() - windowRect.center() + QPoint(windowRect.width()/2, windowRect.height()/2));
    } else {
        // 部分跑出去 → 拉回来
        int newX = qMax(available.left(), qMin(windowRect.left(), available.right() - windowRect.width()));
        int newY = qMax(available.top(),  qMin(windowRect.top(),  available.bottom() - windowRect.height()));
        if (newX != windowRect.left() || newY != windowRect.top()) {
            move(newX, newY);
        }
    }
}

void MainWindow::SetAvatar(QPixmap pix)
{
    if(pix.isNull()) return;

    // 获取按钮的大小
    QSize btnSize = ui->avatar_btn->size();
    QSize btnSize1 = ui->avatar_btn1->size();
    // 加载原图
    QPixmap src(pix);
    QPixmap src1(pix);

    // 缩放到控件大小
    src = src.scaled(btnSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    src1 = src1.scaled(btnSize1, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    // 创建一个透明的画布
    QPixmap dest(btnSize);
    dest.fill(Qt::transparent);
    QPixmap dest1(btnSize1);
    dest1.fill(Qt::transparent);
    // 创建painter
    QPainter painter(&dest);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QBrush(src));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(dest.rect(), btnSize.width()/2, btnSize.width()/2);
    painter.end();

    QPainter painter1(&dest1);
    painter1.setRenderHint(QPainter::Antialiasing, true);
    painter1.setBrush(QBrush(src1));
    painter1.setPen(Qt::NoPen);
    QRect innerRect = dest1.rect().adjusted(3, 3, -3, -3);
    painter1.drawRoundedRect(innerRect, 3, 3);
    painter1.end();

    // 设置给两个button
    ui->avatar_btn->setIcon(QIcon(dest));   // 把 QPixmap 转成 QIcon
    ui->avatar_btn->setIconSize(btnSize);   // 这句必须加！否则图标会很小
    ui->avatar_btn1->setIcon(QIcon(dest1));
    ui->avatar_btn1->setIconSize(btnSize1);
}

QIcon MainWindow::CreateColorDot(QColor color)
{
    QPixmap pix(20, 20); // 画布大小
    pix.fill(Qt::transparent); // 透明底
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
    painter.setBrush(QBrush(color)); // 设置填充色
    painter.setPen(Qt::NoPen);       // 不要边框
    // 画圆  前两个是左上角 后两个是直径
    painter.drawEllipse(6, 6, 8, 8);
    return QIcon(pix);
}

void MainWindow::UploadAvatarToServer(const QPixmap& pix)
{
    if(pix.isNull()) return;
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, "PNG");
    // 转成Base64字符串
    QString base64 = bytes.toBase64();

    m_sock->SendSaveAvatarRequest(m_username, base64);
}

void MainWindow::DownloadAvatarToServer()
{
    m_sock->SendGetAvatarRequest(m_username, true);
}

void MainWindow::InitUi()
{
    m_isBlack = true;

    m_username = m_info.username;
    ui->name_lb->setText(m_username);
    m_balance = m_info.balance;
    ui->balance_lb->setText(QString::number(m_info.balance, 'f', 2));
    m_billDlg->SetBalance(m_balance);

    ui->username_edit2->setAlignment(Qt::AlignCenter); // 居中
    ui->realname_edit2->setAlignment(Qt::AlignCenter);
    ui->phone_edit2->setAlignment(Qt::AlignCenter);

    ui->name_lb->setProperty("styleType", "text");
    ui->name_lb->style()->unpolish(ui->name_lb);
    ui->name_lb->style()->polish(ui->name_lb);
    ui->balance_lb->setProperty("styleType", "text");
    ui->balance_lb->style()->unpolish(ui->name_lb);
    ui->balance_lb->style()->polish(ui->name_lb);
    ui->money_lb->setProperty("styleType", "text");
    ui->money_lb->style()->unpolish(ui->money_lb);
    ui->money_lb->style()->polish(ui->money_lb);

    ui->report_widget->SetUi();
    ui->payment_widget->SetUi();

    QListView *view = new QListView(ui->building);
    view->setAttribute(Qt::WA_TranslucentBackground);
    view->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    view->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    view->setCursor(Qt::PointingHandCursor);
    ui->building->setView(view);
    ui->building->setStyleSheet(
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 6px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(255, 255, 255, 40);"
        "    border-radius: 3px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(255, 255, 255, 80);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );

    // 设置按像素平滑滚动，而不是按行跳变
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->verticalScrollBar()->setSingleStep(5); // 5像素




    // 去掉点击时的虚线框
    ui->chatRoom_widget->setFocusPolicy(Qt::NoFocus);
    // 或者用 ui->listChat->setAttribute(Qt::WA_MacShowFocusRect, false);

    ui->chat_widget->setStyleSheet(
        "QLabel#fandou_lb {" // 聊天室的标题样式
        "    color: white;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "    padding: 5px 10px;"
        "    background-color: rgba(255, 255, 255, 10);"
        "    border-top-left-radius: 20px;"
        "    border-top-right-radius: 20px;"
        "    border-bottom: 1px solid rgba(255, 255, 255, 20);"
        "}"
    );
    // 聊天室
    ui->chatRoom_widget->setWordWrap(true); // 开启文字自动换行
    ui->chatRoom_widget->setSpacing(5);     // 消息和消息之间留点缝隙
    ui->chatRoom_widget->setStyleSheet(
        "QListWidget#chatRoom_widget {"
        "    background-color: transparent;"
        "    border-radius: 20px;"
        "    outline: none;"
        "    color: #ffffff;"
        "}"
        "QListWidget#chatRoom_widget:item {"
        "    background-color: transparent;"
        "}"
        // 滚动条美化
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 6px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(255, 255, 255, 40);"
        "    border-radius: 3px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(255, 255, 255, 80);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );
    ui->chatRoom_widget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->chatRoom_widget->verticalScrollBar()->setSingleStep(7);

    // 消息输入框
    ui->sendMsg_Edit->setPlaceholderText("请输入消息...");
    ui->sendMsg_Edit->setStyleSheet(
        "QTextEdit {"
        "    background-color: rgba(255, 255, 255, 30);"
        "    border: 1px solid rgba(255, 255, 255, 80);"
        "    border-radius: 20px;"
        "    color: #eeeeee;" /* 文字颜色 */
        "    padding: 4px;"
        "    font-size: 14px;"
        "}"
        "QTextEdit:focus {"
        "    border: 1px solid #e0e0e0;"
        "    background-color: rgba(0, 0, 0, 70);"
        "}"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: rgba(255,255,255,40); border-radius: 2px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );
    ui->sendMsg_Edit->verticalScrollBar()->setSingleStep(3);

    SetGroupSendBtn();
}

void MainWindow::SetCursorStyle()
{
    ui->home_btn->setCursor(Qt::PointingHandCursor);
    ui->pay_btn->setCursor(Qt::PointingHandCursor);
    ui->notes_btn->setCursor(Qt::PointingHandCursor);
    ui->report_btn->setCursor(Qt::PointingHandCursor);
    ui->personal_btn->setCursor(Qt::PointingHandCursor);
    ui->notice_list->setCursor(Qt::PointingHandCursor);
    ui->close_btn->setCursor(Qt::PointingHandCursor);
    ui->mini_btn->setCursor(Qt::PointingHandCursor);
    ui->changeAvatar_btn->setCursor(Qt::PointingHandCursor);
    ui->building->setCursor(Qt::PointingHandCursor);
    ui->man_rb->setCursor(Qt::PointingHandCursor);
    ui->woman_rb->setCursor(Qt::PointingHandCursor);
    ui->logOut_btn->setCursor(Qt::PointingHandCursor);
    ui->saveChange_btn->setCursor(Qt::PointingHandCursor);
    ui->cancel_btn->setCursor(Qt::PointingHandCursor);
    ui->send_btn->setCursor(Qt::PointingHandCursor);
    ui->payAll_btn->setCursor(Qt::PointingHandCursor);
    ui->waterPay_btn->setCursor(Qt::PointingHandCursor);
    ui->electPay_btn->setCursor(Qt::PointingHandCursor);
    ui->gasPay_btn->setCursor(Qt::PointingHandCursor);
    ui->parkPay_btn->setCursor(Qt::PointingHandCursor);
    ui->propertyPay_btn->setCursor(Qt::PointingHandCursor);
}

void MainWindow::InitPersonalData()
{
    // 主界面构造的时候就获取用户信息了，这里设置一下
    ui->username_edit2->setText(m_username);
    ui->realname_edit2->setText(m_info.realname);
    ui->phone_edit2->setText(m_info.phone);
    ui->man_rb->setChecked(false);
    ui->woman_rb->setChecked(false);
    if(m_info.sex == 1){
        ui->man_rb->setChecked(true);
    }
    else{
        ui->woman_rb->setChecked(true);
    }
    ui->building->setCurrentText(m_info.building);
    ui->floor->setValue(m_info.floor);
    ui->houseNum->setValue(m_info.housenum);
}

void MainWindow::AddChatMessage(QString sender, QString content, QString time, bool isPrivate)
{
    // 在聊天室界面显示聊天信息
    QWidget* bubbleWidget = nullptr;
    QString appPath = QCoreApplication::applicationDirPath();
    bool isSelf = false;
    if(sender == m_username) isSelf = true;
    else isSelf = false;
    // 实例化UI类
    if (isSelf) {
        RightBubble* right = new RightBubble(this);
        right->SetText(content);
        QString appPath = QCoreApplication::applicationDirPath();
        QString imagePath = appPath + "/cache/avatar_" + m_username + ".png";
        right->SetAvatar(imagePath);
        bubbleWidget = right;
        qDebug() << "自己" << content;
        if(isPrivate) right->SetContentColor(QColor::fromRgb(255, 0, 0));
    }
    else {
        LeftBubble* left = new LeftBubble(this);
        // 检查本地有没有发送人的头像，没有就用默认的

        QString appPath = QCoreApplication::applicationDirPath();
        QString imagePath = appPath + "/cache/avatar_" + sender + ".png";
        QFileInfo fileInfo(imagePath);
        //(这里大概率不会触发，因为socket里有检查有没有头像，没有就立即请求投降了，这个没测试)
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            imagePath = ":/image/image/default_avatar.png";
        }
        left->SetAvatar(imagePath);
        QString info = sender + " " + time;
        left->SetTime(info);
        left->SetText(content);
        left->SetSender(sender);
        bubbleWidget = left;
        qDebug() << "别人" << content;
        if(isPrivate) left->SetContentColor(QColor::fromRgb(255, 0, 0));
        connect(left, &LeftBubble::privateSend, this, &MainWindow::onPrivateSend);
    }

    QListWidgetItem* item = new QListWidgetItem(ui->chatRoom_widget);

    int itemWidth = ui->chatRoom_widget->viewport()->width();
    itemWidth = 229 - 22;
    int itemHeight = bubbleWidget->sizeHint().height();
    bubbleWidget->setFixedWidth(itemWidth);
    bubbleWidget->adjustSize();
    item->setSizeHint(QSize(itemWidth, itemHeight));

    // 放入列表
    ui->chatRoom_widget->setItemWidget(item, bubbleWidget);
    ui->chatRoom_widget->scrollToBottom();
}

QJsonArray MainWindow::ImagesToBase64(const QStringList &imagePaths)
{
    QJsonArray imgJsonArray;

    for (const QString &path : imagePaths) {
        // 加载图片
        QImage image(path);
        if (image.isNull()) continue;

        // 压缩图！
        // 如果图片宽或高超过 1024，就等比缩放到 1024，防止数据量太大
        if (image.width() > 1024 || image.height() > 1024) {
            image = image.scaled(1024, 1024, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // 转为二进制
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);

        // 保存为JPG格式
        image.save(&buffer, "JPG", 60);

        // 转成Base64字符串
        QString base64Str = QString::fromLatin1(byteArray.toBase64());

        imgJsonArray.append(base64Str);
    }
    return imgJsonArray;
}

void MainWindow::GetProcessProgress()
{
    m_sock->SendGetProgressRequest(m_username);
}

void MainWindow::SaveAvatarToLocal(const QPixmap& pix, const QString& sender)
{
    QString appPath = QCoreApplication::applicationDirPath();
    // 保存图片到本地(例如: ./cache/avatar_yeji.png)
    QString imgPath;
    if(sender == "") imgPath = appPath + "/cache/avatar_" + m_username + ".png";
    else imgPath = appPath + "/cache/avatar_" + sender + ".png";
    if (!pix.isNull()) {
        pix.save(imgPath, "PNG");
    }
}

void MainWindow::GetBillsInfo()
{
    m_sock->SendGetBillsInfoRequest(m_username);
}

void MainWindow::SetGroupSendBtn()
{
    ui->send_btn->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 10);" /* 很淡的白色背景 */
        "    border: 1px solid rgba(255, 255, 255, 30);"
        "    image: url(:/image/image/send.png);"
        "    border-radius: 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 80);"
        "    border: 1px solid rgba(255, 255, 255, 100);"
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(255, 255, 255, 100);" /* 按下更深 */
        "    padding-top: 2px; padding-left: 2px;" /* 简单的按下位移感 */
        "}"
    );
}

void MainWindow::SetPrivateSendBtn()
{
    ui->send_btn->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 10);" /* 很淡的白色背景 */
        "    border: 1px solid rgba(255, 255, 255, 30);"
        "    image: url(:/image/image/private_send.png);"
        "    border-radius: 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 80);"
        "    border: 1px solid rgba(255, 255, 255, 100);"
        "    color: white;"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(255, 255, 255, 100);" /* 按下更深 */
        "    padding-top: 2px; padding-left: 2px;" /* 简单的按下位移感 */
        "}"
    );
}

void MainWindow::AddNoticeItem(const NoticeInfo &info)
{
    QListWidgetItem* item = new QListWidgetItem(ui->notice_list);
    item->setText(info.title);
    item->setIcon(CreateColorDot(info.typeColor)); // 设置代码画出来的图标
    QVariant var;
    var.setValue(info);
    item->setData(Qt::UserRole, var);
}

void MainWindow::ShowContentDialog(const NoticeInfo &info)
{

    m_ntcDlg = new NoticeDialog(this);
    m_ntcDlg->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
    m_ntcDlg->setWindowModality(Qt::ApplicationModal);

    // 把数据传入dialog
    m_ntcDlg->SetLogContent(info.title, info.content, info.sender, info.time, info.typeColor);

    m_ntcDlg->show();
}
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 算出鼠标相对于窗口的偏移量
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::onUpdateAvatar(bool success, QString message, QString avatar, QString sender)
{
    if(success){
        QPixmap pix;
        QByteArray imgBytes = QByteArray::fromBase64(avatar.toUtf8());
        pix.loadFromData(imgBytes);
        if(sender == "") SaveAvatarToLocal(pix);
        else if(!pix.isNull()) SaveAvatarToLocal(pix, sender);
        // 如果数据库中没头像,不处理
        SetAvatar(pix);
    }
    else{
        qDebug() << message;
    }
}

void MainWindow::onUpdateNotice(bool success, QString message, const QList<NoticeInfo> &list)
{
    if(success){
        qDebug() << "获取公告成功：" << message;
        ui->notice_list->clear();
        for(const NoticeInfo& info : list){
            AddNoticeItem(info);
        }
    }
    else{
        qDebug() << "获取公告失败：" << message;
    }
}

void MainWindow::onUpdateProgress(bool success, QString message, const QList<ReportInfo> &list)
{
    if(success){
        qDebug() << "获取处理进度成功：" << message;
        ui->report_widget->clearAll();
        for(const ReportInfo& info : list){
            ui->report_widget->addReportItem(info);
        }
    }
    else{
        qDebug() << "获取处理进度失败：" << message;
    }
}

void MainWindow::onUpdateBills(bool success, QString message, const QList<BillInfo>& list)
{
    if(success){
        qDebug() << "获取账单成功：" << message;
        for(const BillInfo& info : list){
            if(info.title == "water_info"){
                m_waterSpend = info.spend;
                m_waterCost = info.spend * info.subInfo.toDouble();
                QString s_spend = "¥" + QString::number(m_waterCost, 'f', 2); // 保留两位小数
                QString s_info = "已使用:" + QString::number(info.spend) + "·m³";
                ui->water_widget->SetData("水费", s_spend, s_info, DashboardCard::Style_Chart, info.dataList, 0, QColor(52, 152, 219));
                ui->water_widget->SetModifiableData(m_waterCost, s_info); // 传花费多少钱
            }
            else if(info.title == "elect_info"){
                m_electSpend = info.spend;
                m_electCost = info.spend * info.subInfo.toDouble();
                QString s_spend = "¥" + QString::number(m_electCost, 'f', 2);
                QString s_info = "已使用:" + QString::number(info.spend) + "度";
                ui->elect_widget->SetData("电费", s_spend, s_info, DashboardCard::Style_Chart, info.dataList, 0, QColor(241, 196, 15));
                ui->elect_widget->SetModifiableData(m_electCost, s_info);
            }
            else if(info.title == "gas_info"){
                m_gasSpend = info.spend;
                m_gasCost = info.spend * info.subInfo.toDouble();
                QString s_spend = "¥" + QString::number(m_gasCost, 'f', 2);
                QString s_info = "已使用:" + QString::number(info.spend) + "·m³";
                ui->gas_widget->SetData("燃气", s_spend, s_info, DashboardCard::Style_Chart, info.dataList, 0, QColor(231, 76, 60));
                ui->gas_widget->SetModifiableData(m_gasCost, s_info);
            }
            else if(info.title == "heating_info"){
                QString s_spend = "¥25/m²";
                QString s_info = "住房面积:" + info.subInfo + "·m²";
                ui->heating_widget->SetData("暖气", s_spend, s_info, DashboardCard::Style_Bar, {}, info.spend, QColor(230, 126, 34));
            }
            else if(info.title == "parking_info"){
                QDate maturityDate = QDate::fromString(info.subInfo, "yyyy-MM-dd");
                QDate currentDate = QDate::currentDate();
                int daysDiff = currentDate.daysTo(maturityDate); // 差了多少天
                int yearsDiff = maturityDate.year() - currentDate.year(); //  差了多少年
                if(yearsDiff == 0) yearsDiff += 1;
                QString s_spend = "¥" + QString::number(info.spend) + "/年";
                QString s_info;
                if(daysDiff <= 0) s_info = "已过期";
                else s_info = "有效期至:" + info.subInfo;
                m_parkDate = info.subInfo;
                double percent = daysDiff / (365.00 * yearsDiff);
                QColor themeColor;
                if(percent > 0 && percent <= 0.25) themeColor = QColor(255, 0, 0);
                else if(percent > 0.25 && percent <= 0.5) themeColor = QColor(255, 165, 0);
                else if(percent > 0.5 && percent <= 0.75) themeColor = QColor(173, 255, 47);
                else themeColor = QColor(0, 255, 0);
                ui->parking_widget->SetData("停车年卡", s_spend, s_info, DashboardCard::Style_Ring, {}, percent, themeColor);
                ui->parking_widget->SetModifiableData(info.spend, "", &maturityDate); // 传剩余量和日期
            }
            else{
                QDate maturityDate = QDate::fromString(info.subInfo, "yyyy-MM-dd");
                QDate currentDate = QDate::currentDate();
                int daysDiff = currentDate.daysTo(maturityDate);
                int yearsDiff = maturityDate.year() - currentDate.year();
                if(yearsDiff == 0) yearsDiff += 1;
                QString s_spend = "¥" + QString::number(info.spend) + "/年";
                QString s_info;
                if(daysDiff <= 0) s_info = "已过期";
                else s_info = "有效期至:" + info.subInfo;
                m_propertyDate = info.subInfo;
                double percent = daysDiff / (365.00 * yearsDiff);
                QColor themeColor;
                if(percent > 0 && percent <= 0.25) themeColor = QColor(255, 0, 0);
                else if(percent > 0.25 && percent <= 0.5) themeColor = QColor(255, 165, 0);
                else if(percent > 0.5 && percent <= 0.75) themeColor = QColor(173, 255, 47);
                else themeColor = QColor(0, 255, 0);
                ui->property_widget->SetData("物业费", s_spend, s_info, DashboardCard::Style_Ring, {}, percent, themeColor);
                ui->property_widget->SetModifiableData(info.spend, "", &maturityDate);
            }
        }
    }
    else{
        qDebug() << "获取账单失败：" << message;
    }
}

void MainWindow::onUpdatePayment(bool success, QString message, const QList<PaymentInfo> &list)
{
    if(success){
        qDebug() << "获取缴费记录成功：" << message;
        ui->payment_widget->clearAll();
        for(const PaymentInfo& info : list){
            ui->payment_widget->addPaymentItem(info);
        }
    }
    else{
        qDebug() << "获取缴费记录失败：" << message;
    }
}

void MainWindow::onUpdateInfo(bool success, QString message)
{
    if(success){
        m_resDlg = new ResultDialog(this);
        m_resDlg->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
        m_resDlg->setWindowModality(Qt::ApplicationModal);
        m_resDlg->SetMessage(message);
        m_resDlg->show();
    }
    else{
        qDebug() << "personal info get error: " << message;
    }
}

void MainWindow::onDelivery(int type, QString content, int level, QStringList imagePaths, int status, QString time)
{
    ReportInfo info;
    info.username = m_username;
    info.type = type;
    info.content = content;
    info.level = level;
    info.images = ImagesToBase64(imagePaths);
    info.status = status;
    info.time = time;
    m_sock->SendDeliveryRequest(info);
    // 直接加载到下面的列表里
    ui->report_widget->addReportItem(info, true);
}

void MainWindow::onPrivateSend(QString receiver)
{
    m_isPrivate = true;
    m_receiver = receiver;
    SetPrivateSendBtn();
}

void MainWindow::on_changeAvatar_btn_clicked()
{
    QString avatarPath = QFileDialog::getOpenFileName(this, "选择更换头像", "", "图片文件(*.png *.jpg *.jpeg *.bmp)");
    if(avatarPath.isEmpty()) return;

    QPixmap pix(avatarPath);
    if(pix.isNull()) return;
    SaveAvatarToLocal(pix);
    // 更新到界面
    SetAvatar(pix);
    // 发送到服务器
    UploadAvatarToServer(pix);
}

void MainWindow::on_logOut_btn_clicked()
{
    m_sock->SendLogoutRequest(m_username);
    this->hide();
    emit logoutRequested();
    qApp->exit(773);
    this->close();
}

void MainWindow::on_cancel_btn_clicked()
{
    InitPersonalData();
}

void MainWindow::on_saveChange_btn_clicked()
{
    m_info.username = ui->username_edit2->text();
    m_info.realname = ui->realname_edit2->text();
    m_info.phone = ui->phone_edit2->text();
    int s = ui->man_rb->isChecked() ? 1 : 2;
    m_info.sex = s;
    m_info.building = ui->building->currentText();
    m_info.floor = ui->floor->value();
    m_info.housenum = ui->houseNum->value();
    // 发送数据
    m_sock->SendChangeInfoRequest(m_info);
}

void MainWindow::on_send_btn_clicked()
{
    QString content = ui->sendMsg_Edit->toPlainText().trimmed();
    // 空的不发
    if (content.isEmpty()) return;
    // 发送
    if(!m_isPrivate) Socket::getInstance().SendGroupMessage(m_username, content);
    else Socket::getInstance().SendPrivateMessage(m_username, m_receiver, content); // 私信发送
    // 清空输入框
    ui->sendMsg_Edit->clear();
    // 焦点回到输入框，方便继续打字
    ui->sendMsg_Edit->setFocus();
    m_isPrivate = false;
    SetGroupSendBtn();
}

void MainWindow::on_dark_light_btn_clicked()
{
    QString qssPath;
    if(m_isBlack){
        qssPath = ":/style/light.qss";
        ui->dark_light_btn->setStyleSheet("image: url(:/image/image/light_dark.png);");
        m_isBlack = false;
    }
    else{
        qssPath = ":/style/dark.qss";
        ui->dark_light_btn->setStyleSheet("image: url(:/image/image/dark_light.png);");
        m_isBlack = true;
    }

    QFile file(qssPath);
    if(file.open(QFile::ReadOnly | QFile::Text)){
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    }
    else{
        qDebug() << "open QSS failed2!";
    }
}

void MainWindow::on_payAll_btn_clicked()
{
    m_billDlg->exec();
}

void MainWindow::ToAddBill(BillInfo& info, bool checked){
    if(checked){
        m_billDlg->AddBill(info.title, info.subInfo, info.spend);
    }
    else{
        m_billDlg->DelBill(info.title, info.spend);
    }
}

void MainWindow::on_waterPay_btn_clicked(bool checked)
{
    BillInfo info = ui->water_widget->GetBillInfo();
    ToAddBill(info, checked);
    if(checked){
        ui->waterPay_btn->setText("取消");
    }
    else{
        ui->waterPay_btn->setText("选择缴费");
    }
}

void MainWindow::on_electPay_btn_clicked(bool checked)
{
    BillInfo info = ui->elect_widget->GetBillInfo();
    ToAddBill(info, checked);
    if(checked){
        ui->electPay_btn->setText("取消");
    }
    else{
        ui->electPay_btn->setText("选择缴费");
    }
}

void MainWindow::on_gasPay_btn_clicked(bool checked)
{
    BillInfo info = ui->gas_widget->GetBillInfo();
    ToAddBill(info, checked);
    if(checked){
        ui->gasPay_btn->setText("取消");
    }
    else{
        ui->gasPay_btn->setText("选择缴费");
    }
}

void MainWindow::on_parkPay_btn_clicked(bool checked)
{
    BillInfo info = ui->parking_widget->GetBillInfo();
    ToAddBill(info, checked);
    if(checked){
        ui->parkPay_btn->setText("取消");
    }
    else{
        ui->parkPay_btn->setText("选择缴费");
    }
}

void MainWindow::on_propertyPay_btn_clicked(bool checked)
{
    BillInfo info = ui->property_widget->GetBillInfo();
    ToAddBill(info, checked);
    if(checked){
        ui->propertyPay_btn->setText("取消");
    }
    else{
        ui->propertyPay_btn->setText("选择缴费");
    }
}

