#include <unistd.h>
#include <string.h>
#include "vputils.h"
#include "vplayer.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "effects/shibatch/eq.h"

char **gargv;
int gargc;
int counter=2;
void next_cb(char *next, void *usr)
{
    if (counter<gargc){
        strcpy(next,gargv[counter++]);
    }

}
int main(int argc, char *argv[])
{
    gargv = argv;
    gargc=argc;
    VPlayer *pl= new VPlayer(next_cb, NULL);
    VPEffectPluginEQ *eq=new VPEffectPluginEQ(100.0);

    pl->setEffectsList((VPEffectPlugin*)eq);
    pl->open(argv[1]);

    while (pl->isPlaying()) {
        sleep(3);
        pl->pause();
        sleep(1);
        pl->play();
        sleep(2);
    }
    pl->removeEffect((VPEffectPlugin*)eq);
    delete pl;
    delete eq;
    return 0;
}
