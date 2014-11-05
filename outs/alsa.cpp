/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/



#include "vrok.h"
#include "alsa.h"
#define IN_EVENT_SIZE  ( sizeof (struct inotify_event) )
#define IN_EVENT_BUF_LEN     ( 10 * ( IN_EVENT_SIZE + 256 ) )

static const snd_pcm_uframes_t PERIOD_SIZE = 256;
char buffer[IN_EVENT_BUF_LEN] __attribute__ ((aligned(8)));
std::queue<int> inotify_q;


VPOutPlugin* VPOutPluginAlsa::VPOutPluginAlsa_new()
{
    return (VPOutPlugin *) new VPOutPluginAlsa();
}

void VPOutPluginAlsa::worker_run(VPOutPluginAlsa *self)
{
    int ret;
    unsigned out_frames=0;
    unsigned chans=self->bin->chans;
    int *out_buf_i=self->out_buf_i;
    float *out_buf=self->out_buf;
    int multiplier = self->multiplier;
    float *buffer[2];
    buffer[0] = self->bin->buffer[0];
    buffer[1] = self->bin->buffer[1];

    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(self->in_fd, &rfds);

  //  pollfd pfd = { self->in_fd, POLLIN, 5 };

    while (ATOMIC_CAS(&self->work,true,true)){
        if (ATOMIC_CAS(&self->pause_check,true,true)) {
            ATOMIC_CAS(&self->paused,false,true);
            snd_pcm_pause(self->handle,true);
            self->m_pause.lock();
            self->m_pause.unlock();
            snd_pcm_pause(self->handle,false);
            ATOMIC_CAS(&self->paused,true,false);
            ATOMIC_CAS(&self->pause_check,true,false);
            if (!ATOMIC_CAS(&self->work,false,false)) {
                break;
            }

        }

        self->rd.end_of_input = 0;
        self->rd.data_out = self->out_buf;
        self->rd.output_frames = self->out_frames;
        self->rd.output_frames_gen = 1;
        out_frames=0;

        self->owner->mutex[1].lock();

        self->rd.input_frames = VPBUFFER_FRAMES ;
        self->rd.data_in = self->bin->nextBuffer();


        while (self->rd.output_frames_gen) {
            src_process(self->rs,&self->rd);

            self->rd.input_frames -= self->rd.input_frames_used;
            self->rd.data_in += self->rd.input_frames_used*chans;
            out_frames+=self->rd.output_frames_gen;
        }

        for (int i=0;i<out_frames*chans;i++){
            out_buf_i[i]=out_buf[i]*multiplier;
        }
        ret = snd_pcm_writei(self->handle,
                             out_buf_i,
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

void VPOutPluginAlsa::check_contention(void *user)
{
    VPOutPluginAlsa *self=(VPOutPluginAlsa *)user;
    load_inotify(self->in_fd);
    while (inotify_q.size() > 1) {
       inotify_q.pop();
       inotify_q.pop();
    }
    if (!inotify_q.empty()) {
        int m=inotify_q.front();
        if (m == IN_OPEN){
            self->owner->pause();
        } else if ( m==IN_CLOSE ){
            self->owner->play();
        } else {
            DBG("error");
        }
        inotify_q.pop();
    }
}

void VPOutPluginAlsa::load_inotify(int fd)
{
    int len=read( fd, buffer, IN_EVENT_BUF_LEN );
    int i=0;
    while (i<len){
        struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
        if (event->mask & IN_OPEN) {
            inotify_q.push(IN_OPEN);
        }
        if (event->mask & IN_CLOSE) {
            inotify_q.push(IN_CLOSE);
        }

        i += IN_EVENT_SIZE + event->len;
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
        exit(0);
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
    snd_pcm_format_mask_t *mask;
    snd_pcm_format_mask_alloca(&mask);
    snd_pcm_hw_params_get_format_mask(params, mask);
    if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S32))
    {
        DBG("bit depth is 32");
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32);
        multiplier = 1<<31 -1;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S24))
    {
        DBG("bit depth is 24");
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24);
        multiplier = 1<<23 -1;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S16))
    {
        DBG("bit depth is 16");
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16);
        multiplier = 1<<15 -1;
    }
    else if (snd_pcm_format_mask_test(mask, SND_PCM_FORMAT_S8))
    {
        DBG("bit depth is 8");
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S8);
        multiplier = 1<<7 -1;;
    }

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

    out_buf = (float *)ALIGNED_ALLOC(sizeof(float)*out_frames*bin->chans);
    out_buf_i = (int *)ALIGNED_ALLOC(sizeof(int)*out_frames*bin->chans);

    DBG("target rate "<<out_srate);
    work = true;
    paused = false;
    pause_check = false;

    FULL_MEMORY_BARRIER;
    in_fd = inotify_init();
    if ( in_fd < 0 ) {
        DBG("error initializing inotify, auto pause won't work");
    } else {
        in_wd[0]=inotify_add_watch( in_fd, "/dev/snd/pcmC0D0p", IN_OPEN | IN_CLOSE );
    }
    fcntl(in_fd, F_SETFL, O_NONBLOCK);

    worker = new std::thread((void(*)(void*))worker_run, this);
    worker->high_priority();
    DBG("alsa thread made");
    DBG((void *)VPOutPluginAlsa::check_contention);
    VPEvents::getSingleton()->schedulerAddJob((VPEvents::VPJob) VPOutPluginAlsa::check_contention, this,0);
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

    inotify_rm_watch( in_fd, in_wd[0] );

    close(in_fd);

    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }

    snd_pcm_close(handle);

    ALIGNED_FREE(out_buf);
    ALIGNED_FREE(out_buf_i);
    src_delete(rs);

}
