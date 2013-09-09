#include "eqwidget.h"
#include "ui_eqwidget.h"

EQWidget::EQWidget(VPEffectPluginEQ *eq, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::EQWidget)
{
    ui->setupUi(this);
    plugin = eq;
    target_sliders[0] = new QSlider(Qt::Vertical);
    target_sliders[0]->setMaximum(1000);
    target_sliders[0]->setMinimum(-1000);
    target_sliders[0]->setValue((plugin->getPreamp()-1.0f)*1000.0f);
    target_sliders[0]->setProperty("index",QVariant(0));
    connect(target_sliders[0],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));
    ui->lyUserBars->addWidget(target_sliders[0]);

    for (int i=1;i<19;i++){
        if (i){
            target_sliders[i] = new QSlider(Qt::Vertical);
            target_sliders[i]->setMaximum(1000);
            target_sliders[i]->setMinimum(-1000);
            target_sliders[i]->setValue((plugin->getTargetBands()[i-1]-1.0f)*300.0f);
            target_sliders[i]->setProperty("index",QVariant(i));


            connect(target_sliders[i],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));

            ui->lyUserBars->addWidget(target_sliders[i]);

        }


    }


}

void EQWidget::target_changed(int)
{
    QSlider *th = (QSlider *)sender();
    int h=th->property("index").toInt();
    if(h>0)
        plugin->setTargetBand(h-1,th->value()/300.0f+1.0f);
    else
        plugin->setPreamp(th->value()/1000.0f+1.0f);

}

EQWidget::~EQWidget()
{
    for (int i=0;i<19;i++){
        delete target_sliders[i];
    }
    delete ui;
}
