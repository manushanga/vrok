#include <unistd.h>

#include <QApplication>
#include "vrokmain.h"
#include "vplayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VrokMain w;
    w.show();
    VPlayer v((char *)"vrok",0);
    v.setURL((char *)"/media/ENT/Songs/Audios/English/Paul_van_Dyk-Evolution-[VAN2050B]-WEB-320kbps-2012/09-paul_van_dyk_feat_tyler_michaud_and_fisher-all_the_way.mp3");
    v.setVolume(100);
    v.play();
    sleep(200);
    v.stop();
    return a.exec();
}
