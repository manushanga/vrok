
#include "out_alsa.h"

static const char *name="Advanced Linux Sound Architecture";
static const snd_pcm_uframes_t PERIOD_SIZE = 512;

static void worker_run(VPOutPluginAlsa *self)
{
    int ret;
    while (self->work){

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

        if (ret < 0){
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
    snd_pcm_prepare (handle);
    snd_pcm_start (handle);
}
void VPOutPluginAlsa::pause()
{
    snd_pcm_drop (handle);
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
        DBG("Alsa:init: failed to pcm params");
        return -1;
    }
    worker = new std::thread(worker_run, this);
    work=true;
    return 0;
}

unsigned VPOutPluginAlsa::getSamplerate()
{
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0){
        DBG("Alsa:getSamplerate: failed to open pcm");
        return 0;
    }
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    unsigned s;
    if (snd_pcm_hw_params_get_rate_max(params, &s, 0) > -1){
        return s;
    } else {
        return 0;
    }

}
unsigned VPOutPluginAlsa::getChannels()
{
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0){
        DBG("Alsa:getChannels: failed to open pcm");
        return 0;
    }
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    unsigned c;
    if (snd_pcm_hw_params_get_channels_max(params, &c) > -1) {
        return c;
    } else {
        return 0;
    }
}
int VPOutPluginAlsa::finit()
{
    work=false;
    worker->join();
    delete worker;
    worker=NULL;
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    return 1;
}
