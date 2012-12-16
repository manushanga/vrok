/* VPlayer
 *
 */

#ifndef VPLAYER_H
#define VPLAYER_H

#include <thread>
#include <mutex>

#include "vputils.h"

enum VPState_t{VP_IDLE, VP_OPEN, VP_BUFFER, VP_PLAY, VP_PAUSE, VP_STOP, VP_END, VP_ERROR};

class VPOutPlugin;
class VPEffectPlugin;

class VPlayer
{

public:
    static const unsigned BUFFER_FRAMES = 128;

    float *buffer1;
    float *buffer2;

    // mutexes[0..1] for buffer1, mutexes[2..3] for buffer2
    std::mutex *mutexes[4];
    // play, pause, stop event control
    std::mutex *mutex_play;

    std::mutex *mutex_pause;
    bool work;
    bool paused;
    bool stopped;
    bool effects;

    std::thread *play_worker;
    VPEffectPlugin *vpeffect;
    VPOutPlugin *vpout;

    unsigned prev_track_samples;
    unsigned prev_track_channels;
    unsigned track_samples;
    unsigned track_channels;

    VPlayer();
    void prepare();
    virtual int open(char *url) = 0;
    static void play_work(VPlayer *self);
    virtual void reader() = 0;
    int play();// = 0;
    void pause();// = 0;
    void stop();// = 0;
    void post_process(float *buffer);
    void ended();
    virtual int setVolume(unsigned vol) = 0;
    virtual unsigned long getLength() = 0;
    virtual void setPosition(unsigned long t) = 0;
    virtual unsigned long getPosition() = 0;
    virtual ~VPlayer();
};


#endif // VPLAYER_H
