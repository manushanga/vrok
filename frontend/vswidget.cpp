#include "vswidget.h"
#include "ui_vswidget.h"

#include <QPaintEvent>
#include <QPainter>

VSWidget::VSWidget(DockManager *manager, VPEffectPluginVis *vis, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    plugin(vis),
    disp(vis),
    ui(new Ui::VSWidget)
{
    ui->setupUi(this);

    ui->verticalLayout->addWidget(&disp);
    connect(&q,SIGNAL(timeout()),this,SLOT(process()));
    connect(&disp,SIGNAL(doubleClicked()),this,SLOT(dispDoubleClicked()));
    plugin->minimized(false);

    q.setSingleShot(false);
    q.setInterval(40);
    q.start();
}

VSWidget::~VSWidget()
{
    q.stop();

    plugin->minimized(true);

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
    if (plugin->getType() == VPEffectPluginVis::SPECTRUM)
        plugin->setType(VPEffectPluginVis::SCOPE);
    else
        plugin->setType(VPEffectPluginVis::SPECTRUM);

}


void VDisplay::resizeEvent(QResizeEvent *e)
{
    padw=width()/2.0 - VISBUFFER_FRAMES/2;

    if (plugin->getType() == VPEffectPluginVis::SCOPE) {
        padh=height()/2.0;
    } else  {
        padh=height();
    }
}

void VDisplay::paintEvent(QPaintEvent *e)
{
    ATOMIC_CAS(&plugin->filled,true,true);
    float *pbars=plugin->getBars();
    QPainter pp(this);
    pp.setRenderHints(QPainter::Antialiasing, true);
    pp.fillRect(e->rect(),QColor(255,255,255));
    if (!pbars){
        pp.end();
        return;
    }
    register int x=(VPBUFFER_FRAMES /(4*VPBUFFER_PERIOD));
    register int y=x*8;
    register int z=current*VISBUFFER_FRAMES;
    if (plugin->getType() == VPEffectPluginVis::SCOPE) {
        for (register int i=0;i<(VISBUFFER_FRAMES)-1;i+=1){
            pp.drawLine(padw +i*1,padh+pbars[i+z]*padh,padw+ i+1,padh+pbars[i+z+1]*padh);

        }
    } else {
        QPolygon a;

        a.append(QPoint(padw,padh));
        for (register int i=0;i<VISBUFFER_FRAMES;i++){
            bars[i]=pbars[i+z];
        }
        rdft(VISBUFFER_FRAMES,1,bars,ip,w);

        register int j=0;

        for (register int f=0;f<(VISBUFFER_FRAMES);f=f+1+f/115,j+=2){
            bars_cumm[f]=(bars_cumm[f]+log10(1.0f+fabs(bars[f])))/2.0f;
            a.append(QPoint(padw+j,padh*(1.0f-bars_cumm[f])));
        }
        pp.setBrush(*brush);
        a.append(QPoint(padw+j,padh));
        pp.drawPolygon(a);
    }

    pp.end();

    current++;

    if (current == subbuffers){
        ATOMIC_CAS(&plugin->filled,true,false);
        current=0;
    }
}

void VDisplay::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit doubleClicked();
    if (plugin->getType() == VPEffectPluginVis::SCOPE) {
        padh=height()/2.0;
    } else  {
        padh=height();
    }
}
