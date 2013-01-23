#include "eqwidget.h"
#include "ui_eqwidget.h"

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
        sliders[i]->setMinimumHeight(50);
        if (i){
            target_sliders[i] = new QSlider(Qt::Vertical);
            target_sliders[i]->setMaximum(1000);
            target_sliders[i]->setMinimum(-1000);
            target_sliders[i]->setValue((plugin->getTargetBands()[i-1]-1.0f)*300.0f);
            target_sliders[i]->setProperty("index",QVariant(i));

            levels[i-1]=new QProgressBar();
            levels[i-1]->setOrientation(Qt::Vertical);
            levels[i-1]->setMaximumWidth(10);
            levels[i-1]->setFormat("");
            levels[i-1]->setRange(0,100);
            sliders[i]->setValue((plugin->getBands()[i-1]-1.0f)*300.0f);
            sliders[i]->setProperty("index",QVariant(i));
            connect(sliders[i],SIGNAL(valueChanged(int)),this,SLOT(changed(int)));
            connect(target_sliders[i],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));
            ui->lyBars->addWidget(sliders[i]);
            ui->lyUserBars->addWidget(target_sliders[i]);
            ui->lyUserBars->addWidget(levels[i-1]);

        }
        else{
            sliders[0]->setValue((plugin->getPreamp()-1.0f)*1000.0f);
            sliders[0]->setProperty("index",QVariant(0));
            connect(sliders[i],SIGNAL(valueChanged(int)),this,SLOT(changed(int)));
            ui->lyBars->addWidget(sliders[0]);

        }

    }
    tx = new QTimer();
    tx->setSingleShot(false);
    tx->setInterval(300);
    connect(tx,SIGNAL(timeout()),this,SLOT(process()));
    tx->start();
}
void EQWidget::process()
{
    for (unsigned i=0;i<plugin->getBarCount();i++){
        levels[i]->setValue(plugin->getMids()[i]);
    }
    for (unsigned i=0;i<plugin->getBarCount();i++){
        sliders[i+1]->setValue((plugin->getBands()[i]-1.0f)*300.0f);
    }

}
void EQWidget::target_changed(int)
{
    QSlider *th = (QSlider *)sender();
    int h=th->property("index").toInt();
    if(h)
        plugin->setTargetBand(h,th->value()/300.0f+1.0f);

}
void EQWidget::changed(int)
{
    QSlider *th = (QSlider *)sender();
    int h=th->property("index").toInt();
    if (h==0)
        plugin->setPreamp(th->value()/1000.0f+1.0f);
}
EQWidget::~EQWidget()
{
    DBG("");
    for (int i=0;i<18;i++){
        delete sliders[i];
        delete levels[i];
        if (i)
            delete target_sliders[i];
    }
    delete sliders[18];
    delete tx;
    delete ui;
}
