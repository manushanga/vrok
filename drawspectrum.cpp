#include "drawspectrum.h"
#include "effect.h"
#include <unistd.h>

DrawSpectrum::DrawSpectrum( VPlayer *v, QWidget *gvparent, QObject *parent) :
    QObject(parent)
{
    work = false;
    vis=new VPEffectPluginVis(bars);
    v->addEffect((VPEffectPlugin *)vis);
    gs = new QGraphicsScene();
    QBrush z(Qt::darkGreen);
    QPen x(Qt::darkGreen);

    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        gbars[i] = new QGraphicsRectItem(i*11,0,10,0);
        gbars[i]->setBrush(z);
        gbars[i]->setPen(x);
        gs->addItem(gbars[i]);
    }
    gv = new QGraphicsView(gvparent);
    gv->setScene(gs);
}

QGraphicsScene *DrawSpectrum::getScene()
{
    return gs;
}

void DrawSpectrum::process()
{
    while (work){
        vis->mutex_vis.lock();
        for (int i=0;i<VPEffectPluginVis::BARS;i++){
            gbars[i]->setRect(i*14,0,10,bars[i]*-1.2f);
        }
        vis->mutex_vis.unlock();
        gs->update(0.0f,0.0f,100.0f,-100.0f);
        gv->viewport()->update();
        usleep(23000);
    }
    emit finished();
}
DrawSpectrum::~DrawSpectrum()
{
    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        delete gbars[i];
    }

    if (gs)
        delete gs;
    if (vis)
        delete vis;
}
