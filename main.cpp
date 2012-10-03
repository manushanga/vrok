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
    v.setURL((char *)"/media/ENT/Songs/Audios/English/Others/11-uptown-girl.mp3");
    v.play();
    sleep(20);
    v.stop();
    return a.exec();
}
