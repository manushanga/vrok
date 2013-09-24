/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <unistd.h>

#include "vrok.h"
#include "alsa.h"

static const snd_pcm_uframes_t PERIOD_SIZE = 256;

VPOutPlugin* VPOutPluginAlsa::VPOutPluginAlsa_new()
{
    return (VPOutPlugin *) new VPOutPluginAlsa();
}

void VPOutPluginAlsa::worker_run(VPOutPluginAlsa *self)
{
    int ret;
    unsigned out_frames=0;
    unsigned chans=self->bin->chans;

    int *cursor = self->bin->cursor;
    float *buffer[2];
    buffer[0] = self->bin->buffer[0];
    buffer[1] = self->bin->buffer[1];

    while (ATOMIC_CAS(&self->work,true,true)){
        if (ATOMIC_CAS(&self->pause_check,true,true)) {
            ATOMIC_CAS(&self->paused,false,true);
            self->m_pause.lock();
            self->m_pause.unlock();
            ATOMIC_CAS(&self->paused,true,false);
            ATOMIC_CAS(&self->pause_check,true,false);
            if (!ATOMIC_CAS(&self->work,false,false)) {
                break;
            }

        }

        self->rd.end_of_input = 0;
        self->rd.data_out = self->out_buf;
        self->rd.input_frames = VPBUFFER_FRAMES;
        self->rd.output_frames = self->out_frames;
        self->rd.output_frames_gen = 1;
        out_frames=0;

        self->owner->mutex[1].lock();

        self->rd.data_in = self->bin->buffer[1-(*self->bin->cursor)];


        while (self->rd.output_frames_gen) {
            src_process(self->rs,&self->rd);

            self->rd.input_frames -= self->rd.input_frames_used;
            self->rd.data_in += self->rd.input_frames_used*chans;
            out_frames+=self->rd.output_frames_gen;
        }

        ret = snd_pcm_writei(self->handle,
                             self->out_buf,
                             out_frames);

        self->owner->mutex[0].unlock();

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
    if (!ATOMIC_CAS(&paused,false,false) ){
        m_pause.lock();
        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

void __attribute__((optimize("O0"))) VPOutPluginAlsa::resume()
{
    if (ATOMIC_CAS(&paused,false,false) ){
        m_pause.unlock();
        while (ATOMIC_CAS(&paused,false,false)) {}
    }
}
void __attribute__((optimize("O0"))) VPOutPluginAlsa::pause()
{
    if (!ATOMIC_CAS(&paused,false,false) ){
        m_pause.lock();
        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

int VPOutPluginAlsa::init(VPlayer *v, VPBuffer *in)
{
    DBG("Alsa:init");
    owner = v;
    bin = in;
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

    snd_pcm_hw_params_set_channels(handle, params, bin->chans);

    snd_pcm_hw_params_set_period_size(handle, params, PERIOD_SIZE, 0);

    if (snd_pcm_hw_params(handle, params) < 0) {
        DBG("Alsa:init: failed to set pcm params");
       return -1;
    }

    snd_pcm_hw_params_current(handle, params);
    int dir;
    snd_pcm_hw_params_get_rate(params, &out_srate, &dir);
    in_srate = bin->srate;
    int rerr;
    rs = src_new(SRC_SINC_FASTEST, bin->chans, &rerr);
    if (!rs){
        DBG("SRC error"<<rerr);
        return -1;
    }

    rd.src_ratio = (out_srate*1.0)/(in_srate*1.0);
    out_frames = (VPBUFFER_FRAMES*rd.src_ratio)*2;
    out_buf = (float *)malloc(out_frames*sizeof(float)*bin->chans);
    DBG("target rate "<<out_srate);
    work = true;
    paused = false;
    pause_check = false;

    worker = new std::thread((void(*)(void*))worker_run, this);
    DBG("alsa thread made");
    return 0;
}

VPOutPluginAlsa::~VPOutPluginAlsa()
{
    ATOMIC_CAS(&work,true,false);
    // make sure decoders have properly ended then mutex[0] should locked and
    // mutex[1] unlocked from the decoder and mutex[1] locked by output thread
    // we unlock it here to avoid deadlock
    owner->mutex[1].unlock();
    resume();

    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }

    snd_pcm_close(handle);

    free(out_buf);
    src_delete(rs);

}
