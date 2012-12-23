#ifndef WAVEOUT_H
#define WAVEOUT_H

#include "../vplayer.h"
#include "../out.h"

class VPOutPluginWaveOut : public VPOutPlugin {
public:
    VPlayer *owner;
    std::thread *worker;
    bool work;
    bool paused;

    virtual int init(VPlayer *v, unsigned samplerate, unsigned channels);
    virtual void resume();
    virtual void pause();
    virtual unsigned get_samplerate();
    virtual unsigned get_channels();
    virtual int finit();
};

VPOutPlugin* _VPOutPluginWaveOut_new();

#endif // WAVEOUT_H
