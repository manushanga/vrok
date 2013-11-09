#include "vswidget.h"
#include "ui_vswidget.h"

#include <QPaintEvent>
#include <QPainter>

VSWidget::VSWidget(DockManager *manager, VPEffectPluginVis *vis, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    plugin(vis),
    type(VPEffectPluginVis::SPECTRUM),
    disp(VPEffectPluginVis::SPECTRUM,vis->getBars()),
    ui(new Ui::VSWidget)
{
    ui->setupUi(this);

    plugin->setType(type);

    ui->verticalLayout->addWidget(&disp);
    connect(&q,SIGNAL(timeout()),this,SLOT(process()));
    connect(&disp,SIGNAL(doubleClicked()),this,SLOT(dispDoubleClicked()));
    q.setSingleShot(false);
    q.setInterval(40);
    q.start();
}

VSWidget::~VSWidget()
{
    q.stop();
    delete ui;
}

void VSWidget::process()
{
    if (plugin->wstate)
        return;
    disp.update();
}

void VSWidget::dispDoubleClicked()
{
    if (type == VPEffectPluginVis::SPECTRUM)
        type = VPEffectPluginVis::SCOPE;
    else
        type = VPEffectPluginVis::SPECTRUM;

    plugin->setType(type);
    disp.setType(type);
}


void VDisplay::resizeEvent(QResizeEvent *e)
{
    padw=width()/2 - VPBUFFER_FRAMES/8 ;
    padh=height()/2;
}

void VDisplay::paintEvent(QPaintEvent *e)
{
    QPainter pp(this);

    pp.fillRect(e->rect(),QColor(255,255,255));
    if (type == VPEffectPluginVis::SCOPE) {
        for (int i=0,j=0;i<(VPBUFFER_FRAMES-8);i+=8,j+=2){
            pp.drawLine(padw +j,padh+bars[i]*50,padw+ j+2,padh+bars[i+8]*50);
        }
    } else {
        float p=0.01f;
        for (int i=0,j=0;i<(VPBUFFER_FRAMES-8);i+=8,j+=2){
            pp.drawLine(padw +j,padh+bars[i]*2*log10(p),padw+ j+2,padh+bars[i+8]*2*log10(p));
            p+=5.0f;
        }
    }

    pp.end();
}

void VDisplay::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (type==VPEffectPluginVis::SPECTRUM)
        type=VPEffectPluginVis::SCOPE;
    else
        type=VPEffectPluginVis::SPECTRUM;

    emit doubleClicked();
}
