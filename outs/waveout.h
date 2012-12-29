#ifndef WAVEOUT_H
#define WAVEOUT_H

#include <windows.h>
#include <mmsystem.h>

#include "../vplayer.h"
#include "../out.h"

class VPOutPluginWaveOut : public VPOutPlugin {
public:
    VPlayer *owner;
    std::thread *worker;
    std::mutex m_pause;
    bool work;
    bool paused;
    bool pause_check;
    short *wbuffer1;
    short *wbuffer2;
    virtual int init(VPlayer *v, unsigned samplerate, unsigned channels);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual unsigned get_samplerate();
    virtual unsigned get_channels();
    virtual ~VPOutPluginWaveOut();
};

VPOutPlugin* _VPOutPluginWaveOut_new();

#endif // WAVEOUT_H
