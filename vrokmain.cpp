#include "vrokmain.h"
#include "ui_vrokmain.h"
#include "vplayer.h"

VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);
    vp = new VPlayer((char *)"vrok",0);
    vp->setURL((char *)"/media/ENT/Songs/Audios/English/Daughtry - Break The Spell 2011/05 - Crazy.mp3");
    vp->setVolume(100);
    vp->play();

}

VrokMain::~VrokMain()
{
    vp->stop();
    delete ui;
}
