#include <unistd.h>

#include "vputils.h"
#include "vplayer.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "effects/eq.h"


int main(int argc, char *argv[])
{
    VPlayer pl(NULL, NULL);
    VPEffectPluginEQ eq(100.0);

    pl.addEffect((VPEffectPlugin*)&eq);
    pl.open(argv[1]);
    pl.play();

    while (pl.isPlaying()) {
        sleep(1);
    }
    pl.removeEffect((VPEffectPlugin*)&eq);
    return 0;
}
