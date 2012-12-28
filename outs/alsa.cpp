/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include "alsa.h"

static const snd_pcm_uframes_t PERIOD_SIZE = 128;

VPOutPlugin* _VPOutPluginAlsa_new()
{
    return (VPOutPlugin *) new VPOutPluginAlsa();
}

static void worker_run(VPOutPluginAlsa *self)
{
    int ret;
    while (self->work){

        self->owner->mutexes[1].lock();
        ret = snd_pcm_writei(self->handle,
                             self->owner->buffer1,
                             VPlayer::BUFFER_FRAMES);
        self->owner->mutexes[0].unlock();

        self->owner->mutexes[3].lock();
        ret = snd_pcm_writei(self->handle,
                             self->owner->buffer2,
                             VPlayer::BUFFER_FRAMES);
        self->owner->mutexes[2].unlock();
        if (ret == -EPIPE || ret == -EINTR || ret == -ESTRPIPE){
            DBG("trying to recover");
            if ( snd_pcm_recover(self->handle, ret, 0) < 0 ) {
                DBG("recover failed for "<<ret);
            }
        } else if (ret < 0 && ret != -EAGAIN){
            DBG("write error "<<ret);
        }
    }

}


void VPOutPluginAlsa::resume()
{
    if (paused){
        owner->mutexes[3].unlock();
        owner->mutexes[1].unlock();
        paused=false;
    }
}
void VPOutPluginAlsa::pause()
{
    if (!paused){
        while (!owner->mutexes[1].try_lock()) {}
        DBG("should be true"<<owner->mutexes[3].try_lock());

        paused=true;
    }
}

int VPOutPluginAlsa::init(VPlayer *v, unsigned samplerate, unsigned channels)
{
    DBG("Alsa:init");
    owner = v;
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0){
        DBG("Alsa:init: failed to open pcm");
        return -1;
    }
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT);

    snd_pcm_hw_params_set_channels(handle, params, channels);

    snd_pcm_hw_params_set_rate(handle, params, samplerate, 0);

    snd_pcm_hw_params_set_period_size(handle, params, PERIOD_SIZE, 0);

    if (snd_pcm_hw_params(handle, params) < 0) {
        DBG("Alsa:init: failed to set pcm params");
        return -1;
    }

    paused=true;
    work=true;
    worker = new std::thread(worker_run, this);
    DBG("alsa thread made");
    return 0;
}

unsigned VPOutPluginAlsa::get_samplerate()
{
    snd_pcm_t *h;
    if (snd_pcm_open(&h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0){
        DBG("Alsa:getSamplerate: failed to open pcm");
        snd_pcm_close(h);
        return 0;
    }
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(h, params);
    unsigned s;
    if (snd_pcm_hw_params_get_rate_max(params, &s, 0) > -1){
        return s;
    } else {
        return 0;
    }
    snd_pcm_close(h);

}
unsigned VPOutPluginAlsa::get_channels()
{
    snd_pcm_t *h;
    if (snd_pcm_open(&h, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0){
        DBG("Alsa:getChannels: failed to open pcm");
        snd_pcm_close(h);
        return 0;
    }
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(h, params);
    unsigned c;
    if (snd_pcm_hw_params_get_channels_max(params, &c) > -1) {
        return c;
    } else {
        return 0;
    }
    snd_pcm_close(h);

}
VPOutPluginAlsa::~VPOutPluginAlsa()
{
    work=false;
    owner->mutexes[1].unlock();
    owner->mutexes[3].unlock();

    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
}
