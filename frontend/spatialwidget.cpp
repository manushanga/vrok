#include "spatialwidget.h"
#include "ui_spatialwidget.h"

SpatialWidget::SpatialWidget(DockManager *manager,  VPEffectPluginSpatial *sp, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    plugin(sp),
    ui(new Ui::SpatialWidget)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene());
    QPixmap img(":images/effects/spatial/res/diagram.png");
    ui->graphicsView->scene()->addPixmap(img);
    ui->graphicsView->setFixedHeight(img.height() + 10);
    ui->graphicsView->setFixedWidth(img.width() + 10);
    initd=false;

    ui->hsL->setValue((int)(plugin->getParameters()[SPATIAL_PARAM_L]*10000));
    ui->hsH->setValue((int)(plugin->getParameters()[SPATIAL_PARAM_H]*10000));
    ui->hsD->setValue((int)(plugin->getParameters()[SPATIAL_PARAM_D]*100));

    initd=true;
}

void SpatialWidget::registerUi()
{

    dockManager->registerDockWidget(dockWidget, DockManager::Plugin);
    ui->hsL->setValue((int)(plugin->getParameters()[SPATIAL_PARAM_L]*10000));
    ui->hsH->setValue((int)(plugin->getParameters()[SPATIAL_PARAM_H]*10000));
    ui->hsD->setValue((int)(plugin->getParameters()[SPATIAL_PARAM_D]*100));
}


SpatialWidget::~SpatialWidget()
{
    if (initd)
        plugin->setParameters(ui->hsH->value() / 10000.0f, ui->hsL->value() / 10000.0f, ui->hsD->value()/100.0f);
    delete ui;
}

void SpatialWidget::on_hsL_valueChanged(int value)
{
    if (initd)
        plugin->setParameters(ui->hsH->value() / 10000.0f, ui->hsL->value() / 10000.0f, ui->hsD->value()/100.0f);
    ui->lbL->setText(QString::number(value/10000.0)+"m");
}

void SpatialWidget::on_hsH_valueChanged(int value)
{
    if (initd)
        plugin->setParameters(ui->hsH->value() / 10000.0f, ui->hsL->value() / 10000.0f, ui->hsD->value()/100.0f);
    ui->lbH->setText(QString::number(value/10000.0)+"m");
}

void SpatialWidget::on_hsD_valueChanged(int value)
{
    if (initd)
        plugin->setParameters(ui->hsH->value() / 10000.0f, ui->hsL->value() / 10000.0f, ui->hsD->value()/100.0f);
    ui->lbD->setText(QString::number(value/100.0)+"m");
}
