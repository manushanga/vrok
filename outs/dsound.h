#ifndef WAVEOUT_H
#define WAVEOUT_H

#include <windows.h>
#include <dsound.h>

#define _USE_MATH_DEFINES

#include <cmath>

#include "vplayer.h"
#include "out.h"

class VPOutPluginDSound : public VPOutPlugin {
public:

    static LPDIRECTSOUND lpds;
    static LPDIRECTSOUNDBUFFER lpdsbuffer;
    static WAVEFORMATEX wfx;
    static DSBUFFERDESC dsbdesc;
    static HANDLE NotifyEvent[2];
    static LPDIRECTSOUNDNOTIFY lpDsNotify;
    static DSBPOSITIONNOTIFY PositionNotify[2];

    static HRESULT createSoundObject(void);
    static WAVEFORMATEX setWaveFormat(unsigned samplerate , unsigned channels);
    static DSBUFFERDESC setBufferDescription(unsigned size);
    static HRESULT createSecondarySoundBuffer(void);

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
