#include "vswidget.h"
#include "ui_vswidget.h"
#include <cmath>
#include <QPaintEvent>
#include <QPainter>

void VSWidget::cb_playing(void *message, int messageLength, void *user)
{
    ((VSWidget *) user)->q.start();
}

void VSWidget::cb_paused(void *message, int messageLength, void *user)
{
    ((VSWidget *) user)->q.stop();
}

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

    q.setSingleShot(false);

    q.setInterval(40);

    VPEvents::getSingleton()->addListener("StateChangePlaying",(VPEvents::VPListener)cb_playing,this);
    VPEvents::getSingleton()->addListener("StateChangePaused",(VPEvents::VPListener)cb_paused,this);

}

void VSWidget::registerUi()
{
    dockManager->registerDockWidget(dockWidget, DockManager::Plugin);
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

    plugin->setType( (VPEffectPluginVis::vis_t) ((plugin->getType()+1)%3) );

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
    pp.setRenderHints(QPainter::Antialiasing, false);

    if (!pbars){
        pp.end();
        return;
    }

    register int z=current*VISBUFFER_FRAMES;
    if (plugin->getType() == VPEffectPluginVis::SCOPE) {


        pp.fillRect(e->rect(),QColor(255,255,255));

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

        for (register int i=0;i<(VISBUFFER_FRAMES)-1;i+=1){
            pp.drawLine(padw +i*1,padh+pbars[i+z]*padh,padw+ i+1,padh+pbars[i+z+1]*padh);

        }
    } else if (plugin->getType() == VPEffectPluginVis::SPECTRUM_BARS) {

        pp.fillRect(e->rect(),QColor(0,0,0));

        for (register int i=0;i<VISBUFFER_FRAMES;i++){
            bars[i]=(0.54f-0.46f*cos(i*(2*M_PI/VISBUFFER_FRAMES)))*(pbars[i+z]);
        }
        rdft(VISBUFFER_FRAMES,1,bars,ip,w);
        register int j=0;
        register int f;

        pp.setPen(Qt::NoPen);

        for (f=10;f<(VISBUFFER_FRAMES);f=1.1*f,j+=12){
            QPolygon a;

            float tmp= log10(1+fabs(bars[f])) ;
            if (tmp > bars_cumm[f])
                bars_cumm[f]=tmp;
            else
                bars_cumm[f]-=0.04;
            //bars_cumm[f]=(bars_cumm[f]+diff1[f])/2.0f;
            //pp.drawPoint(QPoint(padw+j,padh*(1.0f-bars_cumm[f])-2.0f));
            a.append(QPoint(padw+j,padh));
            a.append(QPoint(padw+j,padh*(1.0f-bars_cumm[f])));
            a.append(QPoint(padw+j+10,padh*(1.0f-bars_cumm[f])));
            a.append(QPoint(padw+j+10,padh));
            pp.setBrush(*brush);

            pp.drawPolygon(a);

        }

    } else if (plugin->getType() == VPEffectPluginVis::SPECTRUM_FIRE) {
        pp.fillRect(e->rect(),QColor(0,0,0));

        for (register int i=0;i<VISBUFFER_FRAMES;i++){
            bars[i]=(0.54f-0.46f*cos(i*(2*M_PI/VISBUFFER_FRAMES)))*(pbars[i+z]);
        }
        rdft(VISBUFFER_FRAMES,1,bars,ip,w);
        register int j=0;
        register int f;
        QPolygon a;

        pp.setBrush(*brush);
        pp.setPen(Qt::NoPen);
        for (f=10;f<(VISBUFFER_FRAMES);f+=2){

            float tmp= log10(1+fabs(bars[f]));
            if (tmp > bars_cumm[f])
                bars_cumm[f]=tmp;
            else
                bars_cumm[f]-=0.07;

            a.append(QPoint(padw+f,padh));
            a.append(QPoint(padw+f,padh*(1.0f-bars_cumm[f])));


        }
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
