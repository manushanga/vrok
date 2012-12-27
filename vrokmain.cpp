/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <unistd.h>

#include "vrokmain.h"
#include "ui_vrokmain.h"
#include "vputils.h"
#include "vplayer.h"
#include <unistd.h>

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "effects/eq.h"
#include "effects/vis.h"

#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

void VrokMain::vis_updater(VrokMain *self)
{/*
    while (self->visuals){
        self->vis->mutex_vis.lock();
        for (int i=0;i<VPEffectPluginVis::BARS;i++){
            self->gbars[i]->setRect(i*14,0,10,self->bars[i]*-1.2f);
        }
        self->vis->mutex_vis.unlock();
        self->gs->update(0.0f,0.0f,100.0f,-100.0f);
        usleep(23000);

        //self->ui->gvBox->viewport()->update();

    }*/
}
VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);

    vp=NULL;
    th=NULL;
    vis=NULL;
    gs = new QGraphicsScene();
    QBrush z(Qt::darkGreen);
    QPen x(Qt::darkGreen);
    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        gbars[i] = new QGraphicsRectItem(i*11,0,10,0);
        gbars[i]->setBrush(z);
        gbars[i]->setPen(x);
        gs->addItem(gbars[i]);
    }
    ui->gvBox->setScene(gs);
    eq =new VPEffectPluginEQ();
    vis = new VPEffectPluginVis(bars);

    vp = new VPlayer();
        vp->addEffect((VPEffectPlugin *) vis);
        vp->addEffect((VPEffectPlugin *) eq);
}


void VrokMain::on_btnPause_clicked()
{
    vp->pause();
}
void VrokMain::on_btnPlay_clicked()
{

    vp->play();
}
void VrokMain::on_btnOpen_clicked()
{
    if (th){
        visuals =false;
        th->join();
        delete th;
        th=NULL;
    }


    ui->txtFile->setText(QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Supported Files (*.flac *.mp3 *.ogg)")));



    vp->open((char *)ui->txtFile->text().toUtf8().data() );

    vp->effects_active = ui->btnFX->isChecked();
    visuals = true;
    th  = new std::thread((void(*)(void*))VrokMain::vis_updater,this);

}
void VrokMain::on_btnFX_clicked()
{
    if (vp->effects_active)
        vp->effects_active = false;
    else
        vp->effects_active = true;
}
VrokMain::~VrokMain()
{

    if (th){
        visuals=false;
        th->join();
    }
    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        delete gbars[i];
    }
    if (vp)
        delete vp;
    if (gs)
        delete gs;
    if (eq)
        delete eq;
    if (vis)
        delete vis;
    delete ui;
}
