#include "reverbwidget.h"
#include "ui_reverbwidget.h"

ReverbWidget::ReverbWidget(DockManager *manager, VPEffectPluginReverb *rb, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    plugin(rb),
    ui(new Ui::ReverbWidget)
{
    ui->setupUi(this);
	int i;
    for (i=0;plugin->getDelay(i)>0;i++) { }
    ui->slReverbs->setMaximum(i);
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

void ReverbWidget::on_pbAdd_clicked()
{
    if (ui->slReverbs->maximum() < MAX_REVERBS-1) {
        ui->slReverbs->setMaximum(ui->slReverbs->maximum()+1);
    }
    ui->slReverbs->setValue(ui->slReverbs->maximum() );
    ui->pbAdd->setText(QString("Add (") + QString::number(ui->slReverbs->maximum()) + QString(")"));
}
