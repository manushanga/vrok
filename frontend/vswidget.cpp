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
    //ATOMIC_CAS(&plugin->filled,true,true);
    float *pbars=plugin->getBars();
    QPainter pp(this);
    pp.setRenderHints(QPainter::Antialiasing, true);
    pp.fillRect(e->rect(),QColor(255,255,255));
    if (!pbars){
        pp.end();
        return;
    }
    pp.save();
    pp.setBrush(Qt::red);
    for (register int i=0;i<VISBUFFER_FRAMES;i++) {
        if (pbars[i]>0.8f)
        {
            pp.drawRect(padw-12,height()-12,10,10);
            break;
        }
    }
    pp.restore();
    register int z=current*VISBUFFER_FRAMES;
    if (plugin->getType() == VPEffectPluginVis::SCOPE) {
        for (register int i=0;i<(VISBUFFER_FRAMES)-1;i+=1){
            pp.drawLine(padw +i*1,padh+pbars[i+z]*padh,padw+ i+1,padh+pbars[i+z+1]*padh);

        }
    } else {
        QPolygon a;

        a.append(QPoint(padw,padh));
        for (register int i=0;i<VISBUFFER_FRAMES;i++){
            bars[i]=(0.54-0.46*cos(i*(2*M_PI/VISBUFFER_FRAMES)))*(pbars[i+z]);
        }
        rdft(VISBUFFER_FRAMES,1,bars,ip,w);
        register int j=0;
        /*
        for (register int i=0;i<VISBUFFER_FRAMES;i++){
            diff[i]=(log10(1.0f+fabs(bars[i])) - diff[i])/2.0f;
            diff1[i]=(diff[i] - diff1[i])/2.0f;
            diff2[i]=(diff1[i] - diff2[i])/2.0f;
        }*/
        /*
        for (register int i=0;i<VISBUFFER_FRAMES;i++){

        }
        for (register int i=0;i<VISBUFFER_FRAMES;i++){
            for (register int k=0;k<VISBUFFER_FRAMES;k++){
                if (bins[i*VISBUFFER_FRAMES+k])
                    pp.drawPoint(padw+j,2+i);
                j+=2;
            }

            j=0;
        }*/
        for (register int f=0;f<(VISBUFFER_FRAMES);f=f+1+f/115,j+=2){
            QColor xx(int((fabs(diff1[f]))*255.0f)%254,0,0);
            pp.setPen(xx);

            bars_cumm[f]=(bars_cumm[f]+log10(1.0f+fabs(bars[f])))/2.0f;
            //bars_cumm[f]=(bars_cumm[f]+diff1[f])/2.0f;
            //pp.drawPoint(QPoint(padw+j,padh*(1.0f-bars_cumm[f])-2.0f));
            a.append(QPoint(padw+j,padh*(1.0f-bars_cumm[f])));
        }
        pp.setBrush(*brush);
        a.append(QPoint(padw+j,padh));
        pp.drawPolygon(a);
    }

    pp.end();

    current++;

    if (current == subbuffers){
     //   ATOMIC_CAS(&plugin->filled,true,false);
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
