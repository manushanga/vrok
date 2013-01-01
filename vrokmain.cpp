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

VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);

    vp=NULL;
    th=NULL;


    eq =new VPEffectPluginEQ();


    vp = new VPlayer();
    vis = new VPEffectPluginVis(bars,100);
    vp->addEffect((VPEffectPlugin *) vis);
    vp->addEffect((VPEffectPlugin *) eq);


    fileslist=NULL;
    dir=NULL;
    tx = new QTimer(this);
    tx->setSingleShot(false);
    tx->setInterval(50);
    gs = new QGraphicsScene();
    QBrush z(Qt::darkGreen);
    QPen x(Qt::darkGreen);

    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        gbars[i] = new QGraphicsRectItem(i*11,0,10,0);
        gbars[i]->setBrush(z);
        gbars[i]->setPen(x);
        gs->addItem(gbars[i]);
    }
    ui->gv->setScene(gs);

    connect(tx, SIGNAL(timeout()), this, SLOT(process()));

}


void VrokMain::process()
{

    vis->mutex_vis.lock();
    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        gbars[i]->setRect(i*14,0,10,bars[i]*-1.2f);
    }
    vis->mutex_vis.unlock();
    gs->update(0.0f,0.0f,100.0f,-100.0f);
    ui->gv->viewport()->update();
}
void VrokMain::on_btnPause_clicked()
{
    vp->pause();
}
void VrokMain::on_btnPlay_clicked()
{
    vp->play();
}
void VrokMain::on_btnOpenDir_clicked()
{
    QString d = QFileDialog::getExistingDirectory(this, "Select directory","");
    if (dir)
        delete dir;
    dir = new QDir(d);
    dir->setFilter(QDir::Files|QDir::Hidden);
    dir->setNameFilters(QStringList()<<"*.flac"<<"*.mp3"<<"*.ogg");
    QStringList list = dir->entryList();
    if (fileslist)
        delete fileslist;
    fileslist = new QStringListModel(list);
    ui->lvFiles->setModel(fileslist);
}
void VrokMain::on_lvFiles_doubleClicked(QModelIndex i)
{
    QString n(dir->absoluteFilePath(i.data().toString()));
    DBG(n.toStdString());

    vp->open((char *) n.toUtf8().data());
    if (i.row() < fileslist->rowCount()-1 ){

        strcpy (vp->next_track,(char *) (dir->absoluteFilePath(fileslist->index(i.row()+1).data().toString())).toUtf8().data());
    }
    vp->effects_active = ui->btnFX->isChecked();
}
void VrokMain::on_btnOpen_clicked()
{


    ui->txtFile->setText(QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Supported Files (*.flac *.mp3 *.ogg)")));


    if (ui->txtFile->text().length()>0) {
        vp->open((char *)ui->txtFile->text().toUtf8().data() );

        vp->effects_active = ui->btnFX->isChecked();

    }
}
void VrokMain::on_btnFX_clicked()
{

    if (vp->effects_active){
        vp->effects_active = false;
        tx->stop();
    }else{
        vp->effects_active = true;
        tx->start();
    }
}
VrokMain::~VrokMain()
{


    if (vp)
        delete vp;
    if (eq)
        delete eq;

    delete ui;
}
