#include "vswidget.h"
#include "ui_vswidget.h"

VSWidget::VSWidget(DockManager *manager, VPEffectPlugin *vis, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    ui(new Ui::VSWidget)
{
    ui->setupUi(this);
}

VSWidget::~VSWidget()
{
    delete ui;
}
