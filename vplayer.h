/* VPlayer
 * A front-end to libVLC, provides basic media play, pause..etc and equalizer
 * support and output callback hook support.
 *
 */

#ifndef VPLAYER_H
#define VPLAYER_H

#include <vlc/vlc.h>
#include <vlc/libvlc_media_player.h>

typedef libvlc_time_t VPTime;

enum VPState_t{VP_IDLE, VP_OPEN, VP_BUFFER, VP_PLAY, VP_PAUSE, VP_STOP, VP_END, VP_ERROR};

int audio_setup(void **data, char *format, unsigned *rate, unsigned *channels);

class VPlayer
{
private:
    libvlc_instance_t *inst;
    libvlc_media_player_t *mp;
    unsigned op;    
    static void audio_render(void *data, const void *samples, unsigned count, int64_t pts);
public:
    VPlayer(char *portname, unsigned output);
    int setURL(char *url);
    /* Basic player controls */
    int play();
    void pause();
    void stop();
    bool isSeekable();
    int setVolume(int vol);
    VPState_t getState();
    VPTime getLength();
    void setPosition(float t);
    float getPosition();
    ~VPlayer();
};


#endif // VPLAYER_H
