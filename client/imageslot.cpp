#include "imageslot.h"

ImageSlot::ImageSlot(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(80, 80); // 设定固定大小
    // 1. 底层图片 Label
    m_imageLb = new QLabel(this);
    m_imageLb->setGeometry(0, 0, 80, 80);
    m_imageLb->setAlignment(Qt::AlignCenter);
    // 默认样式：虚线框
    m_imageLb->setStyleSheet("border: 3px dashed #cccccc; border-radius: 4px; color: #cccccc;");

    // 2. 删除按钮 (初始隐藏)
    m_delBtn = new QPushButton(this);
    m_delBtn->setFixedSize(16, 16);
    m_delBtn->move(65, 0); // 放在右上角 (100-20=80)
    m_delBtn->setCursor(Qt::PointingHandCursor);
    m_delBtn->hide(); // 默认不显示
    m_delBtn->setStyleSheet(
        "QPushButton {image: url(:/image/image/no_del.png); border-radius: 10px; border: none; font-weight: bold; padding-bottom: 2px; }"
        "QPushButton:hover { image: url(:/image/image/del.png);}"
    );

    connect(m_delBtn, &QPushButton::clicked, this, &ImageSlot::clickDelete);
}

void ImageSlot::SetXY(int x, int y)
{
    this->move(x, y);
}

void ImageSlot::SetPath(const QString &path)
{
    m_path = path;
    if (path.isEmpty()) {
        // === 没图状态：显示加号 ===
        m_imageLb->clear();
        m_imageLb->setStyleSheet("image: url(:/image/image/add.png); border: 3px dashed #cccccc; border-radius: 4px; color: #cccccc;");
        m_delBtn->hide(); // 隐藏删除键
        this->setCursor(Qt::PointingHandCursor);
    }
    else {
        // === 有图状态：显示图片 ===
        m_imageLb->setText("");
        // 使用 border-image 让图片自适应填满
        QPixmap pixmap(path);
        m_imageLb->setPixmap(pixmap.scaled(m_imageLb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_imageLb->setScaledContents(false);
        m_imageLb->setAlignment(Qt::AlignCenter);
        m_imageLb->setStyleSheet(QString("border: none; border-radius: 4px;"));
        m_delBtn->show(); // 显示删除键
        this->setCursor(Qt::ArrowCursor); // 有图了就不显示手型了
    }
}

void ImageSlot::mousePressEvent(QMouseEvent *event)
{
    // 如果当前是空的（显示加号），点击触发上传
    if (m_path.isEmpty()) {
        emit clickUpload();
    }
    // 如果有图，点击可以做预览 TODO
    QWidget::mousePressEvent(event);
}
