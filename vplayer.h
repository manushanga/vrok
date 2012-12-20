
/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
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
    // smaller buffers have less cpu usage and more wakeups
    static const unsigned BUFFER_FRAMES = 256;

    // from 0 to 1
    float volume;

    float *buffer1;
    float *buffer2;

    // mutexes[0..1] for buffer1, mutexes[2..3] for buffer2
    std::mutex *mutexes[4];
    // play, pause, stop event control
    std::mutex *mutex_control;
    bool work;
    bool paused;
    bool effects;

    std::thread *play_worker;
    VPEffectPlugin *vpeffect;
    VPOutPlugin *vpout;

    unsigned prev_track_samplerate;
    unsigned prev_track_channels;
    unsigned track_samplerate;
    unsigned track_channels;

    VPlayer();

    // internal interface
    static void play_work(VPlayer *self);
    void reset();
    void vpout_open();
    void vpout_close();
    virtual void reader() = 0;
    void ended();
    void post_process(float *buffer);

    // external
    virtual int open(const char *url) = 0;
    int play();
    void pause();
    void stop();
    void setVolume(float vol);
    float getVolume();
    bool enqueGapless(const char *url);
    bool isPlaying();
    virtual unsigned long getLength() = 0;
    virtual void setPosition(unsigned long t) = 0;
    virtual unsigned long getPosition() = 0;
    virtual ~VPlayer() ;
};


#endif // VPLAYER_H
