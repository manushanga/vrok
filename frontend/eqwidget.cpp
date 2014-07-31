#include "eqwidget.h"
#include "ui_eqwidget.h"
#include <QFileDialog>
#include <QPainter>
#include <QTextStream>
#include <QVBoxLayout>
#include <QSpacerItem>
#define GAIN_RANGE 28
#define GAIN_HALF 14

EQWidget::EQWidget(DockManager *manager, VPEffectPluginEQ *eq, QWidget *parent) :
    plugin(eq),
    ui(new Ui::EQWidget),
    ManagedDockWidget(manager, this, parent)
{
    ui->setupUi(this);

    target_sliders[0] = new QSlider();
    target_sliders[0]->setTickInterval(1);
    target_sliders[0]->setMaximum(GAIN_RANGE);
    target_sliders[0]->setMinimum(0);
    target_sliders[0]->setValue(GAIN_HALF-log10(plugin->getPreamp())*-20);
    target_sliders[0]->setProperty("index",QVariant(0));
    connect(target_sliders[0],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));
    ui->gridLayout->addWidget(target_sliders[0],0,0);
    ui->gridLayout->addWidget(&labelsBottom[0],1,0);

    QFont f;
    f.setPixelSize(10);

    QFontMetrics fm(f);

    QWidget *wid=new QWidget();
    QVBoxLayout *layout=new QVBoxLayout();
    QLabel *labelM = new QLabel("+" + QString::number(GAIN_HALF) + "dB" );
    labelM->setFont(f);
    layout->addWidget(labelM);
    layout->addSpacerItem(new QSpacerItem(0,1000,QSizePolicy::Minimum,QSizePolicy::Maximum));
    labelM=new QLabel("-" + QString::number(GAIN_HALF) + "dB" );
    labelM->setFont(f);
    layout->addWidget(labelM);

    wid->setLayout(layout);
    ui->gridLayout->addWidget(wid,0,1);


    labelsBottom[0].setFont(f);
    labelsBottom[0].setFixedHeight(fm.height());

    labelsBottom[0].setText("Premap");
    for (int i=1;i<19;i++){


        target_sliders[i] = new QSlider();
        target_sliders[i]->setTickInterval(1);
        target_sliders[i]->setMaximum(GAIN_RANGE);
        target_sliders[i]->setMinimum(0);
        target_sliders[i]->setValue(GAIN_HALF-log10(plugin->getBands()[i-1])*-20);
        target_sliders[i]->setProperty("index",QVariant(i));


        connect(target_sliders[i],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));

        ui->gridLayout->addWidget(target_sliders[i],0,i+1);
        ui->gridLayout->addWidget(&labelsBottom[i],1,i+1);

        labelsBottom[i].setFont(f);

        labelsBottom[i].setFixedHeight(fm.height());
        labelsBottom[i].setText(QString(plugin->getBandNames()[i-1]));



    }
    ui->pbAuto->setChecked(plugin->getAutoPreamp());
}

void EQWidget::registerUi()
{
    dockManager->registerDockWidget(dockWidget, DockManager::Plugin);
}

void EQWidget::target_changed(int)
{
    QSlider *th = (QSlider *)sender();
    DBG(th->value());
    int h=th->property("index").toInt();
    if(h>0)
        plugin->setBand(h-1,pow(10,(GAIN_HALF-th->value())/-20.0) );
    else
        plugin->setPreamp(pow(10,(GAIN_HALF-th->value())/-20.0));

}

EQWidget::~EQWidget()
{
    for (int i=0;i<19;i++){
        delete target_sliders[i];
    }
    delete ui;
}

void EQWidget::on_pbReset_clicked()
{
    target_sliders[0]->setValue(GAIN_HALF);
    for (int i=0;i<18;i++){
        target_sliders[i+1]->setValue(GAIN_HALF);
    }
}

void EQWidget::on_pbLoad_clicked()
{
    QString p = QFileDialog::getOpenFileName(this,"Select Preset",".","All Supported EQ Presets(*.veq *.feq);;Vrok EQ (*.veq);;Foobar2000 EQ (*.feq)");
    if (p.toLower().endsWith("veq")) {
        QFile f(p);
        f.open(QFile::ReadOnly);
        QTextStream ts(&f);
        for (int i=0;i<19;i++){
            int x;
            ts>>x;
            target_sliders[i]->setValue(x);
        }
        f.close();
    } else if (p.toLower().endsWith("feq")) {
        QFile f(p);
        f.open(QFile::ReadOnly);
        QTextStream ts(&f);
        for (int i=1;i<19;i++){
            int x;
            ts>>x;
            target_sliders[i]->setValue( (1.0f+x/20.0f)*GAIN_HALF );
        }
        target_sliders[0]->setValue(GAIN_HALF);
        f.close();
    }
}

void EQWidget::on_pbSave_clicked()
{
    QString p = QFileDialog::getSaveFileName(this,"Save Preset",".","Vrok EQ (*.veq)");
    if (p.size()) {
        QFile f(p);
        f.open(QFile::WriteOnly);
        QTextStream ts(&f);
        for (int i=0;i<19;i++){
            ts<<target_sliders[i]->value()<<" ";
        }
        ts<<"\n";
        f.close();
    }
}

void EQWidget::on_pbAuto_toggled(bool checked)
{

    if (checked) {
        preampHistory = plugin->getPreamp();
        plugin->setAutoPreamp(checked);
    } else {
        plugin->setAutoPreamp(checked);
        plugin->setPreamp(preampHistory);
    }

}
