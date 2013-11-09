#include "eqwidget.h"
#include "ui_eqwidget.h"
#include <QFileDialog>
#include <QPainter>
#include <QTextStream>

EQWidget::EQWidget(DockManager *manager, VPEffectPluginEQ *eq, QWidget *parent) :
    plugin(eq),
    ui(new Ui::EQWidget),
    ManagedDockWidget(manager, this, parent)
{
    ui->setupUi(this);

    target_sliders[0] = new QSlider();
    target_sliders[0]->setMaximum(64);
    target_sliders[0]->setMinimum(0);
    target_sliders[0]->setValue(plugin->getPreamp());
    target_sliders[0]->setProperty("index",QVariant(0));
    connect(target_sliders[0],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));
    ui->gridLayout->addWidget(target_sliders[0],0,0);
    ui->gridLayout->addWidget(&labelsBottom[0],1,0);
    QFont f;
    f.setPixelSize(10);

    QFontMetrics fm(f);
    labelsBottom[0].setFont(f);
    labelsBottom[0].setFixedHeight(fm.height());

    labelsBottom[0].setText("Premap");
    for (int i=1;i<19;i++){
        if (i){
            target_sliders[i] = new QSlider();
            target_sliders[i]->setMaximum(96);
            target_sliders[i]->setMinimum(0);
            target_sliders[i]->setValue(96-log10(plugin->getTargetBands()[i-1])*-20);
            target_sliders[i]->setProperty("index",QVariant(i));


            connect(target_sliders[i],SIGNAL(valueChanged(int)),this,SLOT(target_changed(int)));

            ui->gridLayout->addWidget(target_sliders[i],0,i);
            ui->gridLayout->addWidget(&labelsBottom[i],1,i);

            labelsBottom[i].setFont(f);

            labelsBottom[i].setFixedHeight(fm.height());
            labelsBottom[i].setText(QString(plugin->getBandNames()[i-1]));
        }


    }

}

void EQWidget::target_changed(int)
{
    QSlider *th = (QSlider *)sender();
    int h=th->property("index").toInt();
    if(h>0)
        plugin->setTargetBand(h-1,pow(10,(96-th->value())/-20.0) );
    else
        plugin->setPreamp(th->value());

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
    target_sliders[0]->setValue(32);
    for (int i=0;i<18;i++){
        target_sliders[i+1]->setValue(48);
//        plugin->setTargetBand(i,pow(10,48/-20.0)) ;
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
            target_sliders[i]->setValue( (1.0f+x/20.0f)*48 );
        }
        target_sliders[0]->setValue(32);
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
