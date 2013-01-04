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
#include "config.h"
#include <cstring>

QListView *play_list;
QStringListModel *fileslist;
QDir *dir;
void callback_next(char *mem)
{

    int i =play_list->selectionModel()->selectedRows().first().row();

    if (i+1<fileslist->rowCount()){

        strcpy(mem,(char *)dir->absoluteFilePath(fileslist->index(i+1).data().toString()).toUtf8().data());
        play_list->selectionModel()->select(fileslist->index(i),QItemSelectionModel::Deselect);
        play_list->selectionModel()->select(fileslist->index(i+1),QItemSelectionModel::Select);
    }
}
VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);

    vp=NULL;
    th=NULL;
    ew=NULL;

    eq =new VPEffectPluginEQ();


    vp = new VPlayer(callback_next);
    vis = new VPEffectPluginVis(100);


    play_list=ui->lvFiles;
    fileslist=NULL;
    dir=NULL;
    tx = new QTimer(this);
    tx->setSingleShot(false);

    gs = new QGraphicsScene();
    QBrush z(Qt::darkGreen);
    QPen x(Qt::darkGreen);

    for (int i=0;i<VPEffectPluginVis::BARS;i++){
        gbars[i] = new QGraphicsRectItem(i*11,0,10,0);
        gbars[i]->setBrush(z);
        gbars[i]->setPen(x);
        gs->addItem(gbars[i]);
        bar_vals[i]=0.0f;
    }
    ui->gv->setScene(gs);
    vis_counter = 0;
    if (config_get_lastopen().length()>0) {
        dir = new QDir(config_get_lastopen());
        dir->setFilter(QDir::Files|QDir::Hidden);
        dir->setNameFilters(QStringList()<<"*.flac"<<"*.mp3"<<"*.ogg");
        QStringList list = dir->entryList();
        if (fileslist)
            delete fileslist;
        fileslist = new QStringListModel(list);
        ui->lvFiles->setModel(fileslist);
    }
    tx->setInterval(50);
    config_get_eq_bands(eq->getBands());
    eq->setPreamp(config_get_eq_preamp());
    connect(tx, SIGNAL(timeout()), this, SLOT(process()));
    vp->effects_active = true;
}


void VrokMain::process()
{
    if (vis_counter==vis->getBarSetCount()){
        vis_counter=0;
    }


    float *bars = vis->bar_array + VPEffectPluginVis::BARS*vis_counter;
    for (int b=0;b<VPEffectPluginVis::BARS;b++){
        if (bar_vals[b] < 5.0f && bar_vals[b] > 0.0f)
            bar_vals[b] = 0.0f;
        else if (bar_vals[b] < bars[b])
            bar_vals[b] = bars[b];
        else
            bar_vals[b] -= 3.0f+0.1f*bar_vals[b];
        gbars[b]->setRect(b*14,0,10,bar_vals[b] *-1.0f);
    }

    ui->gv->viewport()->update();

    vis_counter++;
    usleep(500);
}
void VrokMain::on_btnPause_clicked()
{
    tx->stop();
    vp->pause();
}
void VrokMain::on_btnPlay_clicked()
{
    vp->play();
    tx->start();
}
void VrokMain::on_btnOpenDir_clicked()
{
    QString d = QFileDialog::getExistingDirectory(this, "Select directory","");
    config_set_lastopen(d);
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

      //  strcpy (vp->next_track,(char *) (dir->absoluteFilePath(fileslist->index(i.row()+1).data().toString())).toUtf8().data());
    }

}
void VrokMain::on_btnOpen_clicked()
{
    ui->txtFile->setText(QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Supported Files (*.flac *.mp3 *.ogg)")));


    if (ui->txtFile->text().length()>0) {
        vp->open((char *)ui->txtFile->text().toUtf8().data() );
    }
}
void VrokMain::on_btnEQ_clicked()
{
    if (ew){
        delete ew;
    }
    ew = new EQWidget(eq);
    ew->show();
}
void VrokMain::on_btnEQt_clicked()
{
    if (vp->isActiveEffect((VPEffectPlugin *)eq))
       vp->removeEffect((VPEffectPlugin *)eq);
    else
       vp->addEffect((VPEffectPlugin *)eq);
}
void VrokMain::on_btnSpec_clicked()
{
    if (vp->isActiveEffect((VPEffectPlugin *)vis)){
        tx->stop();
        vp->removeEffect((VPEffectPlugin *)vis);
    }
    else {
        vp->addEffect((VPEffectPlugin *)vis);
        tx->start();
    }

}
VrokMain::~VrokMain()
{
    config_set_eq_bands(eq->getBands());
    if (tx)
        delete tx;
    if (vp)
        delete vp;
    if (eq)
        delete eq;
    if (vis)
        delete vis;
    delete ui;
}
