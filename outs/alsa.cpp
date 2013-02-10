/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include "config_out.h"
#include "alsa.h"

static const snd_pcm_uframes_t PERIOD_SIZE = 256;

VPOutPlugin* _VPOutPluginAlsa_new()
{
    return (VPOutPlugin *) new VPOutPluginAlsa();
}

static void worker_run(VPOutPluginAlsa *self)
{
    int ret;
    unsigned out_frames;
    unsigned chans=self->owner->track_channels;

    while (self->work){
        if (self->pause_check) {
            snd_pcm_drain(self->handle);
            snd_pcm_reset(self->handle);
            self->paused=true;
            self->m_pause.lock();
            self->m_pause.unlock();
            self->paused = false;
            snd_pcm_prepare(self->handle);
            snd_pcm_start(self->handle);
            self->pause_check = false;
        }

        self->rd.end_of_input = 0;
        self->rd.data_in = self->owner->buffer1;
        self->rd.data_out = self->out_buf;
        self->rd.input_frames = VPBUFFER_FRAMES;
        self->rd.output_frames = self->out_frames;
        self->rd.output_frames_gen = 1;
        out_frames=0;
        self->owner->mutexes[1].lock();

        while (self->rd.output_frames_gen) {
            src_process(self->rs,&self->rd);

            self->rd.input_frames -= self->rd.input_frames_used;
            self->rd.data_in += self->rd.input_frames_used*chans;
            out_frames+=self->rd.output_frames_gen;
        }
        ret = snd_pcm_writei(self->handle,
                             self->out_buf,
                             out_frames);

        self->owner->mutexes[0].unlock();

        self->rd.end_of_input = 0;
        self->rd.data_in = self->owner->buffer2;
        self->rd.data_out = self->out_buf;
        self->rd.input_frames = VPBUFFER_FRAMES;
        self->rd.output_frames = self->out_frames;
        self->rd.output_frames_gen = 1;
        out_frames=0;

        self->owner->mutexes[3].lock();

        while (self->rd.output_frames_gen) {
            src_process(self->rs,&self->rd);
            self->rd.input_frames -= self->rd.input_frames_used;
            self->rd.data_in += self->rd.input_frames_used*chans;
            out_frames+=self->rd.output_frames_gen;
        }
        ret = snd_pcm_writei(self->handle,
                             self->out_buf,
                             out_frames);

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

void __attribute__((optimize("O0"))) VPOutPluginAlsa::rewind()
{
    if (!paused){
        owner->mutexes[3].unlock();
        owner->mutexes[1].unlock();

        m_pause.lock();
        pause_check = true;
        while (!paused) {
            owner->mutexes[3].unlock();
            owner->mutexes[1].unlock();
        }
        snd_pcm_drop(handle);
        snd_pcm_reset(handle);
    }
}

void __attribute__((optimize("O0"))) VPOutPluginAlsa::resume()
{
    if (paused){
        pause_check = false;
        m_pause.unlock();
        while (paused) {}
    }
}
void __attribute__((optimize("O0"))) VPOutPluginAlsa::pause()
{
    if (!paused){

        m_pause.lock();
        pause_check = true;
        while (!paused) {}
    }
}

int VPOutPluginAlsa::init(VPlayer *v, unsigned samplerate, unsigned channels)
{
    DBG("Alsa:init");
    owner = v;
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NO_AUTO_RESAMPLE) < 0){
        DBG("Alsa:init: failed to open pcm");
        return -1;
    }
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_malloc(&swparams);
    snd_pcm_sw_params_current (handle, swparams);
    snd_pcm_sw_params_set_start_threshold (handle, swparams, VPBUFFER_FRAMES - PERIOD_SIZE);
    snd_pcm_sw_params (handle, swparams);
    snd_pcm_sw_params_free(swparams);

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT);

    snd_pcm_hw_params_set_channels(handle, params, channels);

    snd_pcm_hw_params_set_period_size(handle, params, PERIOD_SIZE, 0);

    if (snd_pcm_hw_params(handle, params) < 0) {
        DBG("Alsa:init: failed to set pcm params");
       return -1;
    }

    snd_pcm_hw_params_current(handle, params);
    int dir;
    snd_pcm_hw_params_get_rate(params, &out_srate, &dir);
    in_srate = samplerate;
    int rerr;
    rs = src_new(SRC_SINC_FASTEST, channels, &rerr);
    if (!rs){
        DBG("SRC error"<<rerr);
    }
    rd.src_ratio = (out_srate*1.0d)/(in_srate*1.0d);
    out_frames = (VPBUFFER_FRAMES*rd.src_ratio)+5;
    out_buf = (float *)malloc(out_frames*sizeof(float)*channels);

    m_pause.unlock();
    pause_check=false;
    paused=false;
    work=true;

    worker = new std::thread((void(*)(void*))worker_run, this);
    DBG("alsa thread made");
    return 0;
}

unsigned VPOutPluginAlsa::get_samplerate()
{
    return 192000;
}
unsigned VPOutPluginAlsa::get_channels()
{
    return 9;
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
    delete out_buf;
    src_delete(rs);
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
}
