/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef OUT_ALSA_H
#define OUT_ALSA_H

#include <alsa/asoundlib.h>
#include <samplerate.h>

#include "vplayer.h"
#include "out.h"

#define ALSA_PCM_NEW_HW_PARAMS_API

class VPOutPluginAlsa : public VPOutPlugin {
public:
    static VPOutPlugin* VPOutPluginAlsa_new();
    static void worker_run(VPOutPluginAlsa *self);
    VPlayer *owner;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    std::thread *worker;
    std::mutex m_pause;
    volatile bool work;
    volatile bool pause_check;
    volatile bool paused;
    unsigned in_srate, out_srate;
    SRC_STATE *rs;
    SRC_DATA rd;
    float *out_buf;
    unsigned out_frames;

    virtual int init(VPlayer *v, unsigned samplerate, unsigned channels);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual ~VPOutPluginAlsa();
};


#endif // OUT_ALSA_H
