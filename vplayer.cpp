#include <unistd.h>

#include <cstdio>

#include "vputils.h"
#include "vplayer.h"
#include "out.h"

int audio_setup(void **data, char *format, unsigned *rate, unsigned *channels)
{
    vpout_plugin_t *new_out = (vpout_plugin_t *)*data;

    /* *sigh* libVLC doesn't support anything else */
    sprintf(format, "S16N");
    *rate = new_out->vpout_plugin_samplerate();
    *channels = new_out->vpout_plugin_channels();
    return 0;
}


VPlayer::VPlayer(char *portname, unsigned output)
{
    const char *args[]={"--verbose=0", NULL};
    inst = libvlc_new (1, args);
    mp = libvlc_media_player_new(inst);

    vpout_init(portname, output);

    libvlc_audio_set_format_callbacks(mp, audio_setup, NULL);
    libvlc_audio_set_callbacks(mp, vpout_run, NULL, NULL, NULL, NULL, vpout_plugin_get(output));
}

int VPlayer::setVolume(int vol)
{
    return libvlc_audio_set_volume(mp, vol);
}

int VPlayer::setURL(char *url)
{
    DBG(url);
    if (url) {
        libvlc_media_t *mi;
        if (url[0]=='/' || url[1]==':')
            mi = libvlc_media_new_path(inst, url);
        else
            mi = libvlc_media_new_location(inst, url);

        libvlc_media_player_set_media(mp, mi);
        libvlc_media_release(mi);

        return 0;
    } else {
        return 1;
    }

}
bool VPlayer::isSeekable()
{
    return libvlc_media_player_is_seekable(mp) != 0;
}
VPTime VPlayer::getLength()
{
    return libvlc_media_player_get_length(mp);
}
VPState_t VPlayer::getState()
{
    libvlc_state_t s =libvlc_media_player_get_state(mp);
    switch (s){
        case libvlc_NothingSpecial:
            return VP_IDLE;
        case libvlc_Opening:
            return VP_OPEN;
        case libvlc_Buffering:
            return VP_BUFFER;
        case libvlc_Playing:
            return VP_PLAY;
        case libvlc_Paused:
            return VP_PAUSE;
        case libvlc_Stopped:
            return VP_STOP;
        case libvlc_Ended:
            return VP_END;
        case libvlc_Error:
            return VP_ERROR;
        default:
            return VP_ERROR;
    }
}
void VPlayer::setPosition(float t)
{
    libvlc_media_player_set_position(mp, t);
}

float VPlayer::getPosition()
{
    return libvlc_media_player_get_position(mp);
}

int VPlayer::play()
{
    return libvlc_media_player_play (mp);
}

void VPlayer::pause()
{
    libvlc_media_player_pause (mp);
}

void VPlayer::stop()
{
    libvlc_media_player_stop (mp);
}
VPlayer::~VPlayer()
{
    vpout_end();
    libvlc_media_player_release (mp);
    libvlc_release (inst);
}

