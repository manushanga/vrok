#include <unistd.h>
#include <string.h>
#include "vputils.h"
#include "vplayer.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "effects/eq.h"

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
    VPlayer pl(next_cb, NULL);
    VPEffectPluginEQ eq(100.0);

    pl.addEffect((VPEffectPlugin*)&eq);
    pl.open(argv[1]);

    while (pl.isPlaying()) {
        sleep(1);
    }
    pl.removeEffect((VPEffectPlugin*)&eq);
    return 0;
}
