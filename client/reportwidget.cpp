#include "reportwidget.h"
#include "ui_reportwidget.h"
#include <QFileDialog>
#include <QStyle>
#include "resultdialog.h"

ReportWidget::ReportWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReportWidget)
{
    ui->setupUi(this);

    InitUi();
    InitStars();
    RefreshSlots();
    m_currentLevel = 0;
    m_resDlg = new ResultDialog(this);
    m_resDlg->setWindowModality(Qt::ApplicationModal);
}

ReportWidget::~ReportWidget()
{
    delete ui;
}

bool ReportWidget::eventFilter(QObject *watched, QEvent *event)
{
    QPushButton* btn = qobject_cast<QPushButton*>(watched);

    // 如果是我们的星星按钮之一
    if (btn && m_btns.contains(btn)) {

        // 获取这颗星星是第几个 (0-4)
        int index = m_btns.indexOf(btn);

        // 鼠标进入
        if (event->type() == QEvent::Enter) {
            UpdateStars(index + 1);
            return true; // 事件已处理
        }

        // 鼠标离开
        if (event->type() == QEvent::Leave) {
            UpdateStars(m_currentLevel);
            return true;
        }
    }

    // 其他事件交给父类默认处理
    return QWidget::eventFilter(watched, event);
}

void ReportWidget::InitUi()
{
    for (int i = 0; i < 5; ++i) {
        ImageSlot* slot = new ImageSlot(this);
        slot->SetXY(i * 110 + 15, 170);
        m_slots.append(slot);
        // 当点击加号时
        connect(slot, &ImageSlot::clickUpload, this, &ReportWidget::onHandleUpload);
        // 当点击删除时
        connect(slot, &ImageSlot::clickDelete, [=](){ onHandleDelete(i); });
    }

    ui->message_tEdit->setPlaceholderText("请详细描述您遇到的问题...");

    // 设置 QSS 样式
    ui->message_tEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: rgba(0, 0, 0, 50);"
        "    /* 极细的白边，模拟玻璃边缘 */"
        "    border: 1px solid rgba(255, 255, 255, 40);"
        "    border-radius: 8px;"
        "    /* 字体：灰白色，留出呼吸感 (padding) */"
        "    color: #E0E0E0;"
        "    padding: 10px;"
        "    font-size: 14px;"
        "}"
        ""
        "QTextEdit:hover {"
        "    border: 1px solid rgba(255, 255, 255, 80);"
        "    background-color: rgba(0, 0, 0, 40);"
        "}"
        ""
        "QTextEdit:focus {"
        "    border: 1px solid #cccccc;"
        "    background-color: rgba(0, 0, 0, 30);"
        "}"
    );
    ui->toReport_btn->setCursor(Qt::PointingHandCursor);
    ui->toComplaint_btn->setCursor(Qt::PointingHandCursor);

    ui->hurryLevel_lb->setProperty("styleType", "text");
    ui->hurryLevel_lb->style()->unpolish(ui->hurryLevel_lb);
    ui->hurryLevel_lb->style()->polish(ui->hurryLevel_lb);

    ui->level_lb->setProperty("styleType", "text");
    ui->level_lb->style()->unpolish(ui->level_lb);
    ui->level_lb->style()->polish(ui->level_lb);

    ui->star_btn1->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
    ui->star_btn2->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
    ui->star_btn3->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
    ui->star_btn4->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
    ui->star_btn5->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
}

void ReportWidget::InitStars()
{
    m_btns << ui->star_btn1 << ui->star_btn2 << ui->star_btn3 << ui->star_btn4 << ui->star_btn5;
    for(int i = 0; i < 5; i++){
        QPushButton* btn = m_btns[i];
        btn->setCursor(Qt::PointingHandCursor);
        btn->installEventFilter(this);
        connect(btn, &QPushButton::clicked, [=](){
            m_currentLevel = i + 1;
            UpdateStars(m_currentLevel);
            m_level = m_currentLevel;
        });
    }
}

void ReportWidget::RefreshSlots()
{
    int currentCount = m_imagePaths.size(); // 当前有几张图
    for (int i = 0; i < 5; ++i) {
        ImageSlot* slot = m_slots[i];

        if (i < currentCount) {
            // 这个位置有图
            // 把第 i 张图显示在第 i 个格子上
            slot->setVisible(true);
            slot->SetPath(m_imagePaths[i]);
        }
        else if (i == currentCount) {
            // 这是紧跟在最后一张图后面的位置
            // 显示“加号”
            slot->setVisible(true);
            slot->SetPath(""); // 空路径 = 加号状态
        }
        else {
            // 隐藏后面的
            slot->setVisible(false);
        }
    }
}

void ReportWidget::UpdateStars(int level)
{
    if(level == 1){
        ui->star_btn1->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/g_star.png)");
        for(int i = 4; i > 0; i--){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
        }
        ui->level_lb->setText("不着急。。。。");
        ui->level_lb->setStyleSheet("color: #4CAF50; font: 12pt \"Microsoft YaHei UI\";");
    }
    else if(level == 2){
        for(int i = 0; i < 2; i++){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/l_star.png)");
        }
        for(int i = 4; i > 1; i--){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
        }
        ui->level_lb->setText("有点着急。。");
        ui->level_lb->setStyleSheet("color: #ADFF2F; font: 12pt \"Microsoft YaHei UI\";");
    }
    else if(level == 3){
        for(int i = 0; i < 3; i++){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/y_star.png)");
        }
        for(int i = 4; i > 2; i--){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
        }
        ui->level_lb->setText("比较着急");
        ui->level_lb->setStyleSheet("color: yellow; font: 12pt \"Microsoft YaHei UI\";");
    }
    else if(level == 4){
        for(int i = 0; i < 4; i++){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/o_star.png)");
        }
        for(int i = 4; i > 3; i--){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
        }
        ui->level_lb->setText("很着急!!");
        ui->level_lb->setStyleSheet("color: orange; font: 12pt \"Microsoft YaHei UI\";");
    }
    else if(level == 5){
        for(int i = 0; i < 5; i++){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/r_star.png)");
        }
        ui->level_lb->setText("急急国王!!!!");
        ui->level_lb->setStyleSheet("color: red; font: 12pt \"Microsoft YaHei UI\";");
    }
    else{
        for(int i = 0; i < 5; i++){
            m_btns[i]->setStyleSheet("border: none; background-color: transparent; image: url(:/image/image/w_star.png)");
        }
        ui->level_lb->setText("");
    }
}

bool ReportWidget::CheckMessage()
{
    QString content = ui->message_tEdit->toPlainText();
    if(content.isEmpty() || m_currentLevel == 0) return false;
    return true;
}

void ReportWidget::InitRequest()
{
    UpdateStars(0);
    m_imagePaths = {}; // 图片路径置空
    RefreshSlots();
    ui->message_tEdit->clear();
}

void ReportWidget::onHandleUpload()
{
    // 限制：如果已经满5张，不让传 (理论上UI会隐藏加号，这里双重保险)
    if (m_imagePaths.size() >= 5) return;

    QString path = QFileDialog::getOpenFileName(this, "选择图片", "", "Images (*.png *.jpg *.jpeg)");
    if (path.isEmpty()) return;

    // 加数据
    m_imagePaths.append(path);

    // 刷新界面 (自动排序、显示下一个加号)
    RefreshSlots();
}

void ReportWidget::onHandleDelete(int index)
{
    // 如果点的格子的索引超出范围
    if (index < 0 || index >= m_imagePaths.size()) return;

    // 删数据
    // 比如有 A B C 删了B List 变成 A C
    m_imagePaths.removeAt(index);
    qDebug() << "delete: " << index;
    // 刷新界面
    RefreshSlots();
}

void ReportWidget::on_toReport_btn_clicked()
{
    if(!CheckMessage()){
        m_resDlg->SetMessage("请把内容填充完整!");
        m_resDlg->setWindowModality(Qt::ApplicationModal);
        m_resDlg->show();
        return;
    }
    QString content = ui->message_tEdit->toPlainText();
    QDateTime currentTime = QDateTime::currentDateTime();
    QString time = currentTime.toString("yyyy-MM-dd hh:mm:ss");
    emit toDelivery(1, content, m_level, m_imagePaths, 0, time);
    InitRequest();
    m_resDlg->SetMessage("报修提交成功!");
    m_resDlg->show();
}

void ReportWidget::on_toComplaint_btn_clicked()
{
    if(!CheckMessage()){
        m_resDlg->SetMessage("请把内容填充完整!");
        m_resDlg->show();
        return;
    }
    QString content = ui->message_tEdit->toPlainText();
    QDateTime currentTime = QDateTime::currentDateTime();
    QString time = currentTime.toString("yyyy-MM-dd hh:mm:ss");
    emit toDelivery(2, content, m_level, m_imagePaths, 0, time);
    InitRequest();
    m_resDlg->SetMessage("投诉提交成功!");
    m_resDlg->show();
}

