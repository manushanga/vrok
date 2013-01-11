#include "eqwidget.h"
#include "ui_eqwidget.h"
#include <QLabel>
EQWidget::EQWidget(VPEffectPluginEQ *eq, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::EQWidget)
{
    ui->setupUi(this);
    plugin = eq;


    for (int i=0;i<19;i++){
        sliders[i] = new QSlider(Qt::Vertical);
        sliders[i]->setMaximum(1000);
        sliders[i]->setMinimum(-1000);
        if (i)
            sliders[i]->setValue((plugin->getBands()[i-1]-1.0f)*300.0f);
        else
            sliders[0]->setValue((plugin->getPreamp()-1.0f)*1000.0f);
        sliders[i]->setProperty("index",QVariant(i));
        connect(sliders[i],SIGNAL(valueChanged(int)),this,SLOT(changed(int)));

        ui->lyBars->addWidget(sliders[i]);
    }
}
void EQWidget::changed(int)
{
    QSlider *th = (QSlider *)sender();
    int h=th->property("index").toInt();
    if (h)
        plugin->setBand(h-1,th->value()/300.0f+1.0f);
    else
        plugin->setPreamp(th->value()/1000.0f+1.0f);
}
EQWidget::~EQWidget()
{
    for (int i=0;i<19;i++)
        delete sliders[i];
    delete ui;
}
