#include "reverbwidget.h"
#include "ui_reverbwidget.h"

ReverbWidget::ReverbWidget(DockManager *manager, VPEffectPluginReverb *rb, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    plugin(rb),
    ui(new Ui::ReverbWidget)
{
    ui->setupUi(this);
    ui->slReverbs->setMaximum(MAX_REVERBS);
    on_slReverbs_sliderMoved(0);
}

ReverbWidget::~ReverbWidget()
{
    delete ui;
}

void ReverbWidget::on_slReverbs_sliderMoved(int position)
{
    ui->slAmplitude->setValue((int)(plugin->getAmplitude(position)*100.0f));
    ui->slDelay->setValue((int)(plugin->getDelay(position)*1000.0f));
}

void ReverbWidget::on_slAmplitude_sliderMoved(int position)
{
    plugin->setAmplitude(ui->slReverbs->value(), position/100.0f );
}

void ReverbWidget::on_slDelay_sliderMoved(int position)
{
    plugin->setDelay(ui->slReverbs->value(),position/1000.0f);
}
