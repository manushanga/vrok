/* VPlayer
 *
 */

#ifndef VPLAYER_H
#define VPLAYER_H

#include <thread>
#include <mutex>

#include "vputils.h"
//#include "out.h"

enum VPState_t{VP_IDLE, VP_OPEN, VP_BUFFER, VP_PLAY, VP_PAUSE, VP_STOP, VP_END, VP_ERROR};

class VPOutPlugin;

class VPlayer
{
public:
    static const unsigned BUFFER_FRAMES = 8192;

    float *buffer1;
    float *buffer2;
    unsigned prev_track_samples;
    unsigned prev_track_channels;
    unsigned track_samples;
    unsigned track_channels;

    /* 01 pq for buffer1
       23 pq for buffer2 */
    std::mutex *mutexes[4];

    VPOutPlugin *vpout;

    void init();
    virtual int open(char *url) = 0;
    virtual int play() = 0;
    virtual void pause()=0;
    virtual void stop()=0;
    virtual int setVolume(unsigned vol)=0;
    virtual unsigned long getLength()=0;
    virtual void setPosition(unsigned long t)=0;
    virtual unsigned long getPosition()=0;
    void end();
};


#endif // VPLAYER_H
