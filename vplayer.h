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

class VPlayer
{
public:
    static const unsigned BUFFER_FRAMES = 8192;

    float *buffer1;
    float *buffer2;

    // mutexes[0..1] for buffer1, mutexes[2..3] for buffer2
    std::mutex *mutexes[4];
    bool work;
    bool paused;
    std::thread *play_worker;
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
    virtual int setVolume(unsigned vol) = 0;
    virtual unsigned long getLength() = 0;
    virtual void setPosition(unsigned long t) = 0;
    virtual unsigned long getPosition() = 0;
};


#endif // VPLAYER_H
