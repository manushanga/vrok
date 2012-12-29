/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef OUT_ALSA_H
#define OUT_ALSA_H

#include <alsa/asoundlib.h>

#include "../vplayer.h"
#include "../out.h"

#define ALSA_PCM_NEW_HW_PARAMS_API

class VPOutPluginAlsa : public VPOutPlugin {
public:
    VPlayer *owner;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    std::thread *worker;
    std::mutex m_pause;
    bool work;
    bool pause_check;
    bool paused;

    virtual int init(VPlayer *v, unsigned samplerate, unsigned channels);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual unsigned get_samplerate();
    virtual unsigned get_channels();
    virtual ~VPOutPluginAlsa();
};

VPOutPlugin* _VPOutPluginAlsa_new();

#endif // OUT_ALSA_H
