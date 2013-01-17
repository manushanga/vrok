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

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "effects/eq.h"
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
    ew=NULL;


    vp = new VPlayer(callback_next);
    eq =new VPEffectPluginEQ(100);

    tx = new QTimer(this);
    tx->setSingleShot(false);
    tx->setInterval(50);
    tx->stop();


    play_list=ui->lvFiles;
    fileslist=NULL;
    dir=NULL;

    gs = new QGraphicsScene();
    QBrush z(Qt::darkGreen);
    QPen x(Qt::darkGreen);

    for (unsigned i=0;i<BAR_COUNT;i++){
        gbars[i] = new QGraphicsRectItem(i*11,0,10,0);
        gbars[i]->setBrush(z);
        gbars[i]->setPen(x);
        gs->addItem(gbars[i]);
        bar_vals[i]=0.0f;
    }
    ui->gv->setScene(gs);

    if (config_get_lastopen().length()>0) {
        dir = new QDir(config_get_lastopen());
        dir->setFilter(QDir::Files|QDir::Hidden);
        dir->setNameFilters(QStringList()<<"*.flac"<<"*.mp3"<<"*.ogg");
        fileslist = new QStringListModel(dir->entryList());
        ui->lvFiles->setModel(fileslist);
    }
    vis_counter = 0;
    connect(tx, SIGNAL(timeout()), this, SLOT(process()));
    vp->effects_active = true;

}


void VrokMain::process()
{
    if (eq->bar_array==NULL)
        return;

    if (vis_counter==eq->getBarSetCount()){
        vis_counter=0;
    }

    float *bars = eq->bar_array + eq->getBarCount()*vis_counter;
    for (unsigned b=0;b<eq->getBarCount();b++){
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
}
void VrokMain::on_btnPause_clicked()
{
    if (ui->btnSpec->isChecked())
        tx->stop();
    vp->pause();
}
void VrokMain::on_btnPlay_clicked()
{
    vp->play();
    if (ui->btnSpec->isChecked())
        tx->start();
}
void VrokMain::on_btnOpenDir_clicked()
{


    QString d = QFileDialog::getExistingDirectory(this, tr("Open Dir"),
                                             "/home",
                                             0);
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
void VrokMain::on_btnEQ_clicked()
{
    if (ew)
        delete ew;
    ew = new EQWidget(eq);
    this->addDockWidget(Qt::BottomDockWidgetArea,ew);
}
void VrokMain::on_btnEQt_clicked()
{
    if (vp->isActiveEffect((VPEffectPlugin *)eq)){
      //  tx->stop();
       vp->removeEffect((VPEffectPlugin *)eq);
    }
    else{
       vp->addEffect((VPEffectPlugin *)eq);
      // tx->start();
           }
}
void VrokMain::on_btnSpec_clicked()
{
    if (vp->isActiveEffect((VPEffectPlugin *)eq)){
        if (ui->btnSpec->isChecked())
            tx->start();
        else
            tx->stop();
    }

}
VrokMain::~VrokMain()
{

    if (vp)
        delete vp;
    if (eq)
        delete eq;
    if (tx)
        delete tx;
    delete ui;
}
