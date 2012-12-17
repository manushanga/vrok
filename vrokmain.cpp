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
#include "players/player_flac.h"
#include "players/player_mpeg.h"
#include <QFileDialog>
VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);
   // //vp = new MPEGPlayer();
   // vp= new FLACPlayer();
  // vp->prepare();
   //vp->open((char *)"/home/madura/Downloads/Lenny Kravitz - Greatest Hits (2000) [FLAC]/02 - Fly Away.flac");
   // vp->play();
    //sleep(5);
    //vp->pause();
    //sleep(5);
    //vp->play();
    //vp->open((char *)"/media/ENT/Dump/Downloads/PSY_-_Gangnam_Style.mp3");
    //vp->open((char *)"/media/ENT/Dump/Downloads/GTA Radio/FLASH.mp3");
    //vp->play();
    //sleep(3);
    //vp->pause();
    //sleep(1);
   // vp->play();
   // sleep(3);
    //vp->stop();
    //sleep(1);
    //vp->play();
    //vp->end();

    ui->txtFile->setText(QString::fromWCharArray(L"/media/ENT/Songs/Audios/සිංහල/Athma Liyanage/Sanda Hiru Thaarakaa.mp3"));
    vp=NULL;
}

void VrokMain::on_btnStop_clicked()
{

    vp->stop();
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
    if (vp)
        delete vp;
    ///ui->txtFile->setText(QFileDialog::getOpenFileName(this, tr("Open File"),
      //                                              "",
      //                                              tr("Supported Files (*.flac *.mp3)")));

    if (ui->txtFile->text().endsWith(".flac",Qt::CaseInsensitive) == true) {
        vp= new FLACPlayer();
    } else {
        vp = new MPEGPlayer();
    }

    vp->open((char *)ui->txtFile->text().toUtf8().data() );
    if (ui->btnFX->isChecked()==true)
        vp->effects = true;

}
void VrokMain::on_btnFX_clicked()
{
    if (vp->effects)
        vp->effects = false;
    else
        vp->effects = true;
}
VrokMain::~VrokMain()
{
    delete ui;
}
