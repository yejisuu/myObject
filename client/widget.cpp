#include "widget.h"
#include "ui_widget.h"
#include <QStyle>
#include <QMessageBox>
#include "clickablelabel.h"
#include <QSettings>
#include <QTimer>


Widget::Widget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(600, 400);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window); // 去掉边框且设置成独立窗口
    this->setAttribute(Qt::WA_TranslucentBackground);

    InitUI();
    InitLineStyle(); // 设置label属性
    SetCursorStyle();
    InitCloseStyle();
    InitLoginSwitch();
    this->setFocus(); // 把光标指向背景
    ui->masterStack->setCurrentIndex(0); // loginPage
    connect(ui->exit_btn0, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->exit_btn1, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->auto_login_checkBox, &QCheckBox::stateChanged, this, [=](int state){
        if (state == Qt::Checked) {
            ui->rember_pwd_checkBox->setChecked(true);
        }
    });
    connect(ui->rember_pwd_checkBox, &QCheckBox::stateChanged, this, [=](int state){
        if (state == Qt::Unchecked) {
            ui->auto_login_checkBox->setChecked(false);
        }
    });
    // 找回密码
    connect(ui->findPwd_lb, &clickablelabel::clicked, this, &Widget::onRetrievePassword);
    // 注册用户
    connect(ui->register_user_lb0, &clickablelabel::clicked, this, &Widget::onRegisterUser);
    connect(ui->register_user_lb1, &clickablelabel::clicked, this, &Widget::onRegisterUser);

    m_sock = &Socket::getInstance();
    m_sock->SetServerConfig("http://127.0.0.1:2552");

    connect(m_sock, &Socket::loginResult, this, &Widget::onLoginResult);
    connect(m_sock, &Socket::registerResult, this, &Widget::onRegisterResult);
    connect(m_sock, &Socket::retrieveResult, this, &Widget::onRetrieveResult);
    connect(m_sock, &Socket::changeResult, this, &Widget::onChangeResult);

    LoadConfig();

    ensureWindowOnScreen();
    connect(qApp, &QGuiApplication::screenAdded, this, &Widget::ensureWindowOnScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &Widget::ensureWindowOnScreen);
}

Widget::~Widget()
{
    SaveConfig();
    delete ui;
}

void Widget::ensureWindowOnScreen()
{
    // 获取当前窗口的完整几何（包括标题栏、边框）
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

void Widget::InitUI()
{
    ui->userName_edit->setPlaceholderText("请输入用户名");
    ui->userName_edit->setAlignment(Qt::AlignCenter); // 居中
    ui->userPwd_edit->setPlaceholderText("请输入密码");
    ui->userPwd_edit->setAlignment(Qt::AlignCenter);
    ui->userPwd_edit->setEchoMode(QLineEdit::Password); // 设置为密码模式

    ui->rootName_edit->setPlaceholderText("请输入管理员账号");
    ui->rootName_edit->setAlignment(Qt::AlignCenter);
    ui->rootPwd_edit->setPlaceholderText("请输入密钥");
    ui->rootPwd_edit->setAlignment(Qt::AlignCenter);
    ui->rootPwd_edit->setEchoMode(QLineEdit::Password); // 设置为密码模式

    ui->loading_lb->setText("正在登录中...");
    ui->loading_lb->hide();
    ui->login_avatar_lb->hide();

    m_loadingMovie = new QMovie(":/image/image/loading.gif");
    ui->loadingGif_lb->setMovie(m_loadingMovie);
    m_loadingMovie->start();
    ui->loadingGif_lb->hide();
}

QPixmap Widget::SetRoundAvatar(const QPixmap& pix, int radius, int borderWidth)
{

    if (pix.isNull()) return QPixmap();

    // --- 参数配置 ---
    int shadowBlur = 20;        // 阴影模糊程度
    int shadowOffset = 8;       // 阴影偏移量 (向右下)
    int borderWid = borderWidth;        // 白圈宽度
    QColor borderColor = Qt::white; // 白圈颜色
    QColor shadowColor(0, 0, 0, 80); // 阴影颜色 (黑色，透明度80)

    // 1. 计算"头像+白圈"的实际半径
    // 为了不让阴影被切掉，头像本体必须比总大小缩小一圈
    // 实际半径 = 总半径 - (阴影偏移 + 模糊余量)
    int mainRadius = radius - (shadowOffset + shadowBlur / 2);
    QSize mainSize(mainRadius * 2, mainRadius * 2);
    // --- 第一步：绘制圆图 + 白圈 (不带阴影) ---
    QPixmap tempPix(mainSize);
    tempPix.fill(Qt::transparent);

    {
        QPainter painter(&tempPix);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        // 绘制路径：圆形
        QPainterPath path;
        path.addEllipse(0, 0, mainSize.width(), mainSize.height());

        // 1.1 裁剪并绘制用户头像
        painter.save();
        painter.setClipPath(path);
        // 缩放原图以填充
        QPixmap scaled = pix.scaled(mainSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 居中绘制
        int x = (mainSize.width() - scaled.width()) / 2;
        int y = (mainSize.height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
        painter.restore();

        // 1.2 画内描边 (白圈)
        // 注意：画笔是居中绘制的，所以要向内缩半个边框宽，否则外边缘会有锯齿
        QPen pen(borderColor, borderWid);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        int offset = borderWid / 2;
        painter.drawEllipse(offset, offset,
                            mainSize.width() - borderWid,
                            mainSize.height() - borderWid);
    }

    // --- 第二步：生成阴影 (利用 QGraphics 体系) ---
    // 这是一个技巧：创建一个虚拟场景，给图片加特效，然后"拍照"
    QGraphicsScene scene;
    QGraphicsPixmapItem* item = scene.addPixmap(tempPix);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlur);
    effect->setOffset(shadowOffset, shadowOffset);
    effect->setColor(shadowColor);
    item->setGraphicsEffect(effect);

    // --- 第三步：输出最终图片 ---
    // 最终大小 = 输入的 radius * 2
    QSize finalSize(radius * 2, radius * 2);
    QPixmap finalPix(finalSize);
    finalPix.fill(Qt::transparent);

    QPainter finalPainter(&finalPix);
    finalPainter.setRenderHint(QPainter::Antialiasing);

    // 将场景渲染到 FinalPix 上，居中放置
    // 为了容纳左上角的阴影扩散(虽然很少)和右下角的偏移，我们需要精确计算渲染矩形
    QRectF sourceRect(-shadowBlur, -shadowBlur, finalSize.width(), finalSize.height());
    scene.render(&finalPainter, QRectF(0,0, finalSize.width(), finalSize.height()), sourceRect, Qt::KeepAspectRatio);

    return finalPix;
}

void Widget::SetCursorStyle()
{
    // 控件格式管理
    ui->login_btn->setCursor(Qt::PointingHandCursor);
    ui->root_login_btn->setCursor(Qt::PointingHandCursor);
    ui->rember_pwd_checkBox->setCursor(Qt::PointingHandCursor);
    ui->auto_login_checkBox->setCursor(Qt::PointingHandCursor);
    ui->rootName_edit->setCursor(Qt::PointingHandCursor);
    ui->rootPwd_edit->setCursor(Qt::PointingHandCursor);
    ui->close_btn->setCursor(Qt::PointingHandCursor);
    ui->mini_btn->setCursor(Qt::PointingHandCursor);
    ui->exit_btn0->setCursor(Qt::PointingHandCursor);
    ui->exit_btn1->setCursor(Qt::PointingHandCursor);
    ui->login_again_btn0->setCursor(Qt::PointingHandCursor);
    ui->login_again_btn1->setCursor(Qt::PointingHandCursor);

}
void Widget::InitLineStyle()
{
    ui->findPwd_lb->setProperty("styleType", "link");
    ui->findPwd_lb->style()->unpolish(ui->findPwd_lb);
    ui->findPwd_lb->style()->polish(ui->findPwd_lb);

    ui->register_user_lb0->setProperty("styleType", "link");
    ui->register_user_lb0->style()->unpolish(ui->register_user_lb0);
    ui->register_user_lb0->style()->polish(ui->register_user_lb0);

    ui->register_user_lb1->setProperty("styleType", "link");
    ui->register_user_lb1->style()->unpolish(ui->register_user_lb1);
    ui->register_user_lb1->style()->polish(ui->register_user_lb1);


    ui->errorText_lb0->setProperty("styleType", "text");
    ui->errorText_lb0->style()->unpolish(ui->errorText_lb0);
    ui->errorText_lb0->style()->polish(ui->errorText_lb0);
    ui->errorText_lb0->setWordWrap(true); // 自动换行
    ui->errorText_lb0->setAlignment(Qt::AlignCenter | Qt::AlignTop); // 让换行的文字顶部对齐

    ui->errorText_lb1->setProperty("styleType", "text");
    ui->errorText_lb1->style()->unpolish(ui->errorText_lb1);
    ui->errorText_lb1->style()->polish(ui->errorText_lb1);
    ui->errorText_lb1->setWordWrap(true); // 自动换行
    ui->errorText_lb1->setAlignment(Qt::AlignCenter | Qt::AlignTop); // 让换行的文字顶部对齐
}
void Widget::InitCloseStyle()
{
    ui->close_btn->setIconSize(QSize(20, 20));
    ui->mini_btn->setIconSize(QSize(20, 20));
    connect(ui->close_btn, &QPushButton::clicked, this, &QWidget::close);
    connect(ui->mini_btn, &QPushButton::clicked, this, &QWidget::showMinimized);
}
void Widget::InitLoginSwitch()
{
    m_loginSwitch = new LoginSwitch(this);
    // 指定选择登录按钮的位置
    m_loginSwitch->move(420, 80);
    m_loginSwitch->setCursor(Qt::PointingHandCursor);
    ui->L_stackedWidget->setCurrentIndex(0);
    connect(m_loginSwitch, &LoginSwitch::selectChanged, this, [=](int index){
        ui->L_stackedWidget->setCurrentIndex(index);
    });
}

void Widget::SaveConfig()
{
    QSettings settings("Remember", "Sys");
    // 记住用户名
    if(!ui->userName_edit->text().isEmpty())
        settings.setValue("username", ui->userName_edit->text());
    bool isRemember = ui->rember_pwd_checkBox->isChecked();
    bool isAuto = ui->auto_login_checkBox->isChecked();
    settings.setValue("isRemember", isRemember);
    settings.setValue("isAuto", isAuto);
    if(isRemember){
        QString enPwd = EncryptPwd(ui->userPwd_edit->text());
        settings.setValue("password", enPwd);
    }
    else{
        settings.remove("password");
    }
}

void Widget::LoadConfig()
{
    QSettings settings("Remember", "Sys");
    QString user = settings.value("username", "").toString();
    if (user.isEmpty()) {
        QPixmap defaultPix(":/image/image/default_avatar.png");
        ui->avatar_lb->setPixmap(SetRoundAvatar(defaultPix, 100, 4));
        ui->login_avatar_lb->setPixmap(SetRoundAvatar(defaultPix, 70, 3));
        return;
    }
    QString appPath = QCoreApplication::applicationDirPath();
    QString imgPath = appPath + "/cache/avatar_" + user + ".png";
    QPixmap userPix(imgPath);
    if (!userPix.isNull()) {
        ui->avatar_lb->setPixmap(SetRoundAvatar(userPix, 100, 4));
        ui->login_avatar_lb->setPixmap(SetRoundAvatar(userPix, 70, 3));
    } else {
        // 文件被删了或者损坏，显示默认
        QPixmap defaultPix(":/image/image/default_avatar.png");
        ui->avatar_lb->setPixmap(SetRoundAvatar(defaultPix, 100, 4));
        ui->login_avatar_lb->setPixmap(SetRoundAvatar(defaultPix, 70, 3));
    }
    ui->avatar_lb->setStyleSheet("background: transparent;");
    ui->login_avatar_lb->setStyleSheet("background: transparent;");

    ui->userName_edit->setText(user);
    bool isRemember = settings.value("isRemember", false).toBool();
    bool isAuto = settings.value("isAuto", false).toBool();
    ui->rember_pwd_checkBox->setChecked(isRemember);
    ui->auto_login_checkBox->setChecked(isAuto);
    if(isRemember){
        QString enPwd = settings.value("password", "").toString();
        QString password = DecryptPwd(enPwd);
        ui->userPwd_edit->setText(password);
    }
    if(isAuto && isRemember && !ui->userPwd_edit->text().isEmpty()){
        QTimer::singleShot(100, this, [=](){
            on_login_btn_clicked();
        });
    }
}
// 密码的加密
QString Widget::EncryptPwd(const QString& raw)
{
    QByteArray data = raw.toUtf8();
    for(int i = 0; i < data.size(); i++){
        data[i] = data[i] ^ 0x5A;
    }
    return data.toBase64();
}
// 密码的解密
QString Widget::DecryptPwd(const QString& enPwd)
{
    QByteArray data = QByteArray::fromBase64(enPwd.toUtf8());
    for(int i = 0; i < data.size(); i++){
        data[i] = data[i] ^ 0x5A;
    }
    return QString::fromUtf8(data);
}

// 鼠标按下
void Widget::mousePressEvent(QMouseEvent *event)
{
    // 只有按鼠标左键才能拖动
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        // 记录鼠标当前位置相对于窗口左上角的偏移量
        // event->globalPosition() 是 Qt6 写法，Qt5 用 event->globalPos()
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
// 鼠标移动
void Widget::mouseMoveEvent(QMouseEvent *event)
{
    // 如果按住左键移动
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        // 移动窗口到新位置
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}
// 鼠标释放
void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging = false;
    QWidget::mouseReleaseEvent(event);
}

// 用户登录
void Widget::on_login_btn_clicked()
{
    ui->masterStack->hide();
    ui->close_btn->hide();
    ui->mini_btn->hide();
    m_loginSwitch->hide();
    ui->avatar_lb->hide();
    ui->login_avatar_lb->show();
    ui->SYS_lb->move(170, 40);
    ui->loading_lb->show();
    ui->loadingGif_lb->show();

    m_isUserLogin = true;
    QString user = ui->userName_edit->text();
    QString pass = ui->userPwd_edit->text();
    if(user.isEmpty() || pass.isEmpty()){
        m_loginSwitch->hide();
        ui->masterStack->setCurrentIndex(1);
        ui->E_stackedWidget->setCurrentIndex(1);
        ui->errorText_lb1->setText("用户名或密码不能为空!");
        return;
    }
    m_sock->SendLoginRequest(MSG_TYPE::USER_LOGIN, user, pass);
}
// 管理员登录
void Widget::on_root_login_btn_clicked()
{
    m_isUserLogin = false;
    QString root = ui->rootName_edit->text();
    QString pass = ui->rootPwd_edit->text();
    if(root.isEmpty() || pass.isEmpty()){
        m_loginSwitch->hide();
        ui->masterStack->setCurrentIndex(1);
        ui->E_stackedWidget->setCurrentIndex(1);
        ui->errorText_lb1->setText("用户名或密码不能为空!");
        return;
    }
    m_sock->SendLoginRequest(MSG_TYPE::ADMIN_LOGIN, root, pass);
}
// 重新登录
void Widget::on_login_again_btn0_clicked()
{
    ui->masterStack->setCurrentIndex(0);
    m_loginSwitch->show();
}
void Widget::on_login_again_btn1_clicked()
{
    ui->masterStack->setCurrentIndex(0);
    m_loginSwitch->show();
}
// 登录结果
void Widget::onLoginResult(bool success, QString message, QJsonObject data)
{
    if (success) {
        // 登录成功
        // 跳转主界面等
        if(m_isUserLogin){
            qDebug() << message;
            QJsonObject userData = data["data"].toObject();
            m_cacheUserInfo.username = data["username"].toString();
            m_cacheUserInfo.realname = data["realname"].toString();
            m_cacheUserInfo.phone = data["phone"].toString();
            m_cacheUserInfo.sex = data["sex"].toInt();
            m_cacheUserInfo.building = data["building"].toString();
            m_cacheUserInfo.floor = data["floor"].toInt();
            m_cacheUserInfo.housenum = data["housenum"].toInt();
            m_cacheUserInfo.balance = data["balance"].toDouble();

            if(m_mainWin){
                m_mainWin->deleteLater();
                m_mainWin = nullptr;
            }

            m_mainWin = new MainWindow(m_cacheUserInfo);
            m_mainWin->hide();

            // 定时，主界面需要提前获取头像等信息
            QTimer::singleShot(3000, this, [this](){
                this->accept();
            });
        }
        else{
            qDebug() << "管理员" << message;
            this->accept();
        }
    }
    else {
        if (m_isUserLogin) {

            QTimer::singleShot(3000, this, [this](){
                ui->masterStack->show();
                ui->close_btn->show();
                ui->mini_btn->show();
                m_loginSwitch->show();
                ui->avatar_lb->show();
                ui->login_avatar_lb->hide();
                ui->SYS_lb->move(40, 40);
                ui->loading_lb->hide();
                ui->loadingGif_lb->hide();
            });

            qDebug() << "居民" << message;
            if(message == "User not found"){ // 未注册
                m_loginSwitch->hide();
                ui->masterStack->setCurrentIndex(1);
                ui->E_stackedWidget->setCurrentIndex(0);
                ui->errorText_lb0->setText("未找到用户，请检查用户名是否输入错误，或点击下方“注册用户”按钮注册!");
            }
            else if(message == "Wrong password"){ // 密码错误
                m_loginSwitch->hide();
                ui->masterStack->setCurrentIndex(1);
                ui->E_stackedWidget->setCurrentIndex(1);
                ui->errorText_lb1->setText("密码错误!!!");
            }
        }
        else {
            qDebug() << "管理员" << message;
            if(message == "Admin not found"){
                m_loginSwitch->hide();
                ui->masterStack->setCurrentIndex(1);
                ui->E_stackedWidget->setCurrentIndex(1);
                ui->errorText_lb1->setText("没有管理员权限!!!");
            }
            else if(message == "Wrong password"){
                m_loginSwitch->hide();
                ui->masterStack->setCurrentIndex(1);
                ui->E_stackedWidget->setCurrentIndex(1);
                ui->errorText_lb1->setText("密钥错误!!!");
            }
        }
    }
}

void Widget::onRegisterResult(bool success, QString message)
{
    m_regDlg->CheckResult(success, message);
}

void Widget::onRetrieveResult(bool success, QString message)
{
    m_retDlg->CheckResult(success, message);
}

void Widget::onChangeResult(bool success, QString message)
{
    m_retDlg->ChangeResult(success, message);
}
// 展示修改窗口
void Widget::onRetrievePassword()
{
    m_retDlg = new RetrieveDialog(this);
    // 关掉就释放对象
    m_retDlg->setAttribute(Qt::WA_DeleteOnClose);
    // 设置成模态窗口
    m_retDlg->setWindowModality(Qt::ApplicationModal);
    int x = this->geometry().x() + (this->width() - m_retDlg->width()) / 2;
    int y = this->geometry().y() + (this->height() - m_retDlg->height()) / 2;
    // 移动过去
    m_retDlg->move(x, y);

    // 发送找回需要的验证数据
    connect(m_retDlg, &RetrieveDialog::sendRetrieveData, this, &Widget::onSendRetrieveData);
    // 发送修改的密码
    connect(m_retDlg, &RetrieveDialog::sendChangePassword, this, &Widget::onSendChangePassword);

    m_retDlg->exec();
}

// 展示注册窗口
void Widget::onRegisterUser()
{
    m_regDlg = new RegisterDialog(this);
    // 关掉就释放对象
    m_regDlg->setAttribute(Qt::WA_DeleteOnClose);
    // 设置成模态窗口
    m_regDlg->setWindowModality(Qt::ApplicationModal);
    int x = this->geometry().x() + ((this->width() - m_regDlg->width()) / 2);
    int y = this->geometry().y() + ((this->height() - m_regDlg->height()) / 2);
    // 移动过去
    m_regDlg->move(x, y);

    // 发送注册数据
    connect(m_regDlg, &RegisterDialog::sendRegisterData, this, &Widget::onSendRegisterData);

    m_regDlg->exec();
}
// 调用注册功能
void Widget::onSendRegisterData(QString username, QString password, QString realname, QString phone)
{
    // 调用socket发送注册数据
    m_sock->SendRegisterRequest(username, password, realname, phone);
}

void Widget::onSendRetrieveData(QString username, QString realname, QString phone)
{
    // 调用socket发送找回数据
    m_sock->SendRetrieveRequest(username, realname, phone);
}

void Widget::onSendChangePassword(QString username, QString password)
{
    // 调用socket发送要修改的密码
    m_sock->SendChangePwdRequest(username, password);
}

void Widget::on_userPwd_edit_returnPressed()
{
    ui->login_btn->click();
}

void Widget::on_rootPwd_edit_returnPressed()
{
    ui->root_login_btn->click();
}

