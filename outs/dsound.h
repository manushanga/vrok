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

    LPDIRECTSOUND lpds;
    LPDIRECTSOUNDBUFFER lpdsbuffer;
    WAVEFORMATEX wfx;
    DSBUFFERDESC dsbdesc;
    HANDLE NotifyEvent[2];
    LPDIRECTSOUNDNOTIFY lpDsNotify;
    DSBPOSITIONNOTIFY PositionNotify[2];

    HRESULT createSoundObject(void);
    WAVEFORMATEX setWaveFormat(unsigned samplerate , unsigned channels);

    static VPOutPlugin* VPOutPluginDSound_new();
    static void worker_run(VPOutPluginDSound *self);
    VPlayer *owner;
    std::thread *worker;
    std::mutex m_pause;
    std::mutex *mutex_pause;
    volatile bool work;
    volatile bool paused;
    volatile bool pause_check;
    volatile unsigned half_buffer_size;
    short *wbuffer;
    virtual int init(VPlayer *v, VPBuffer *in);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual ~VPOutPluginDSound();
};

#endif // WAVEOUT_H
