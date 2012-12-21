
/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
/*
  Notes:
  * Please do not propose to use std::string, I hate it for backend usage.
  * There is no resampling or channel mixing support on the VPlayer and it is
    not planned. If it can't be played on the hardware you have it won't be
    played. Ofcause it is open for anyone else to implement!
*/

#ifndef VPLAYER_H
#define VPLAYER_H

#include <vector>
#include <thread>
#include <mutex>

#include "vputils.h"

enum VPState_t{VP_IDLE, VP_OPEN, VP_BUFFER, VP_PLAY, VP_PAUSE, VP_STOP, VP_END, VP_ERROR};

class VPOutPlugin;
class VPEffectPlugin;

class VPlayer
{
private:
    char *next_track;
    bool gapless_compatible;
    std::vector<VPEffectPlugin *> effects;
public:
    // smaller buffers have less cpu usage and more wakeups
    static const unsigned BUFFER_FRAMES = 255;

    // from 0 to 1
    float volume;

    float *buffer1;
    float *buffer2;

    // mutexes[0..1] for buffer1, mutexes[2..3] for buffer2
    std::mutex *mutexes[4];

    // open, play, pause, stop event control all are considered as cirtical
    // sections, none run interleaved.
    std::mutex *mutex_control;

    // internal, play_worker runs only if work==true, if not it MUST return
    bool work;

    // internal, paused state
    bool paused;

    // external, effects are on if true
    bool effects_active;

    std::thread *play_worker;
    VPOutPlugin *vpout;

    unsigned track_samplerate;
    unsigned track_channels;

    VPlayer();

    // internal interface
    static void play_work(VPlayer *self);
    int vpout_open();
    int vpout_close();
    virtual void reader() = 0;
    void ended();
    void post_process(float *buffer);
    void set_metadata(unsigned samplerate, unsigned channels);

    // external interface
    virtual int open(const char *url) = 0;
    int play();
    void pause();
    void stop();
    void setVolume(float vol);
    float getVolume();
    void addEffect(VPEffectPlugin *eff);
    void removeEffect(unsigned idx);
    bool enqueGapless(const char *url);
    bool isPlaying();
    virtual unsigned long getLength() = 0;
    virtual void setPosition(unsigned long t) = 0;
    virtual unsigned long getPosition() = 0;
    virtual ~VPlayer() ;
};


#endif // VPLAYER_H
