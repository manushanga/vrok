#ifndef OUT_ALSA_H
#define OUT_ALSA_H

#include <alsa/asoundlib.h>

#include "vplayer.h"
#include "out.h"

#define ALSA_PCM_NEW_HW_PARAMS_API

class VPOutPluginAlsa : public VPOutPlugin {
public:
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    std::thread *worker;
    bool work;

    virtual const char *getName();
    virtual int init(unsigned samplerate, unsigned channels);
    virtual void resume();
    virtual void pause();
    virtual unsigned getSamplerate();
    virtual unsigned getChannels();
    virtual int finit();
};

#endif // OUT_ALSA_H
