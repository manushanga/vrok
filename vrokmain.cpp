#include <unistd.h>

#include "vrokmain.h"
#include "ui_vrokmain.h"
#include "vputils.h"
#include "vplayer.h"
#include "players/player_flac.h"

VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);
    vp = new FLACPlayer();
    vp->init();
    vp->open((char *)"/home/madura/Downloads/Lenny Kravitz - Greatest Hits (2000) [FLAC]/02 - Fly Away.flac");
   // vp->play();
    //sleep(5);
    //vp->pause();
    //sleep(5);
    //vp->play();
    //vp->setURL((char *)"/media/ENT/Dump/Downloads/PSY_-_Gangnam_Style.mp3");
    //vp->play();
    //vp->end();
    DBG("asd");
}

VrokMain::~VrokMain()
{
    vp->stop();
    delete ui;
}
