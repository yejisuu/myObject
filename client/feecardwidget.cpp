#include "feecardwidget.h"
#include "ui_feecardwidget.h"

FeeCardWidget::FeeCardWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FeeCardWidget)
{
    ui->setupUi(this);
}

FeeCardWidget::~FeeCardWidget()
{
    delete ui;
}
