/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include "alsa.h"

static const char *name="Advanced Linux Sound Architecture";
static const snd_pcm_uframes_t PERIOD_SIZE = 512;

static void worker_run(VPOutPluginAlsa *self)
{
    int ret;
    while (self->work){
        if (self->paused){
            // we do alsa syncs here, instead of the calling thread
            // because of thread syncing problems
            snd_pcm_drain(self->handle);
            self->mutex_pause->lock();
            snd_pcm_prepare (self->handle);
            snd_pcm_start (self->handle);
        }

        self->owner->mutexes[1]->lock();
        ret = snd_pcm_writei(self->handle,
                             self->owner->buffer1,
                             VPlayer::BUFFER_FRAMES);
        self->owner->mutexes[0]->unlock();

        self->owner->mutexes[3]->lock();
        ret = snd_pcm_writei(self->handle,
                             self->owner->buffer2,
                             VPlayer::BUFFER_FRAMES);
        self->owner->mutexes[2]->unlock();
        if (ret < 0 && ret != -EAGAIN){
            DBG("Alsa:run: write error "<<ret);
        }

    }

}

const char *VPOutPluginAlsa::getName()
{
    return name;
}
void VPOutPluginAlsa::resume()
{
    paused = false;
    mutex_pause->unlock();
}
void VPOutPluginAlsa::pause()
{
    paused = true;
    mutex_pause->try_lock();
}

int VPOutPluginAlsa::init(VPlayer *v, unsigned samplerate, unsigned channels)
{
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
    mutex_pause = new std::mutex();
    mutex_pause->try_lock();
    work=true;
    worker = new std::thread(worker_run, this);

    return 0;
}

unsigned VPOutPluginAlsa::getSamplerate()
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
unsigned VPOutPluginAlsa::getChannels()
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
int VPOutPluginAlsa::finit()
{
    work=false;
    if(!paused)
        pause();
    owner->mutexes[1]->unlock();
    owner->mutexes[3]->unlock();

    resume();
    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
    delete mutex_pause;
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    return 1;
}
