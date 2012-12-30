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
    vp->addEffect((VPEffectPlugin *) eq);
    fileslist=NULL;
    dir=NULL;

    spec = new DrawSpectrum(vp,this);
    th_spec = new QThread();
    spec->work = true;
    spec->moveToThread(th_spec);
    spec->gv->move(10,110);
    spec->gv->resize(290,190);

    connect(th_spec, SIGNAL(started()), spec, SLOT(process()));

    connect(spec, SIGNAL(finished()), th_spec, SLOT(quit()));
    th_spec->start();
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
    if (th){
        visuals =false;
        th->join();
        delete th;
        th=NULL;
    }


    ui->txtFile->setText(QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "",
                                                    tr("Supported Files (*.flac *.mp3 *.ogg)")));


    if (ui->txtFile->text().length()>0) {
        vp->open((char *)ui->txtFile->text().toUtf8().data() );

        vp->effects_active = ui->btnFX->isChecked();
        visuals = true;
        //th  = new std::thread((void(*)(void*))VrokMain::vis_updater,this);
    }
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


    if (vp)
        delete vp;
    if (eq)
        delete eq;

    delete ui;
}
