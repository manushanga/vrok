#include "vrokmain.h"
#include "ui_vrokmain.h"
#include "vplayer.h"

VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);
    vp = new VPlayer((char *)"vrok",0);
    vp->setURL((char *)"/home/madura/Downloads/Lenny Kravitz - Greatest Hits (2000) [FLAC]/02 - Fly Away.flac");
    //vp->setURL((char *)"/media/ENT/Songs/Audios/සිංහල/කසුන් කල්හාර/48 - WORDLESS.mp3");
    vp->setVolume(100);
    vp->play();

}

VrokMain::~VrokMain()
{
    vp->stop();
    delete ui;
}
