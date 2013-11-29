#include "reverbwidget.h"
#include "ui_reverbwidget.h"

ReverbWidget::ReverbWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ReverbWidget)
{
    ui->setupUi(this);
}

ReverbWidget::~ReverbWidget()
{
    delete ui;
}
