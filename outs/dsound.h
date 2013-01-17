#ifndef WAVEOUT_H
#define WAVEOUT_H

#include <windows.h>
#include "dsound.h"

#define _USE_MATH_DEFINES

#include <cmath>

#include "vplayer.h"
#include "out.h"

class VPOutPluginDSound : public VPOutPlugin {
public:
    VPlayer *owner;
    std::thread *worker;
    std::mutex m_pause;
    std::mutex *mutex_pause;
    volatile bool work;
    volatile bool paused;
    volatile bool pause_check;
    volatile unsigned half_buffer_size;
    short *wbuffer;
    virtual int init(VPlayer *v, unsigned samplerate, unsigned channels);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual unsigned get_samplerate();
    virtual unsigned get_channels();
    virtual ~VPOutPluginDSound();
};

VPOutPlugin* _VPOutPluginDSound_new();

#endif // WAVEOUT_H
