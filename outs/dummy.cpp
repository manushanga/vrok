/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <unistd.h>

#include "vrok.h"
#include "dummy.h"


VPOutPlugin* VPOutPluginDummy::VPOutPluginDummy_new()
{
    return (VPOutPlugin *) new VPOutPluginDummy();
}

void VPOutPluginDummy::worker_run(VPOutPluginDummy *self)
{
    int ret;
    unsigned out_frames=0;
    unsigned chans=self->bin->chans;

    while (ATOMIC_CAS(&self->work,true,true)){
        if (ATOMIC_CAS(&self->pause_check,true,true)) {
            ATOMIC_CAS(&self->paused,false,true);
            self->m_pause.lock();
            self->m_pause.unlock();
            ATOMIC_CAS(&self->paused,true,false);
            ATOMIC_CAS(&self->pause_check,true,false);
        }

        self->rd.end_of_input = 0;
        self->rd.data_in = self->bin->buffer;
        self->rd.data_out = self->out_buf;
        self->rd.input_frames = VPBUFFER_FRAMES;
        self->rd.output_frames = self->out_frames;
        self->rd.output_frames_gen = 1;
        out_frames=0;

        self->owner->mutex[1].lock();
        while (self->rd.output_frames_gen) {
            src_process(self->rs,&self->rd);

            self->rd.input_frames -= self->rd.input_frames_used;
            self->rd.data_in += self->rd.input_frames_used*chans;
            out_frames+=self->rd.output_frames_gen;
        }


        self->owner->mutex[0].unlock();

    }

}

void __attribute__((optimize("O0"))) VPOutPluginDummy::rewind()
{
    if (m_pause.try_lock()){
        //m_pause.lock();
        ATOMIC_CAS(&pause_check,false,true);

        owner->mutex[0].lock();
        for (unsigned i=0;i<VPBUFFER_FRAMES*bin->chans;i++)
            bin->buffer[i]=0.0f;
        owner->mutex[1].unlock();

        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

void __attribute__((optimize("O0"))) VPOutPluginDummy::resume()
{
    if (ATOMIC_CAS(&paused,true,true)){
        ATOMIC_CAS(&pause_check,true,false);

        m_pause.unlock();
        while (ATOMIC_CAS(&paused,true,true)) {}
    }
}
void __attribute__((optimize("O0"))) VPOutPluginDummy::pause()
{
    if (!ATOMIC_CAS(&paused,false,false)){

        m_pause.lock();

        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

int VPOutPluginDummy::init(VPlayer *v, VPBuffer *in)
{
    DBG("Dummy:init");
    owner = v;
    bin = in;

    out_srate = 48000;
    in_srate = bin->srate;
    int rerr;
    rs = src_new(SRC_SINC_FASTEST, bin->chans, &rerr);
    if (!rs){
        DBG("SRC error"<<rerr);
        return -1;
    }

    rd.src_ratio = (out_srate*1.0d)/(in_srate*1.0d);
    out_frames = (VPBUFFER_FRAMES*rd.src_ratio)*2;
    out_buf = (float *)malloc(out_frames*sizeof(float)*bin->chans);
    DBG("target rate "<<out_srate);
    work = true;
    paused = false;
    pause_check = false;

    worker = new std::thread((void(*)(void*))worker_run, this);
    DBG("Dummy thread made");
    return 0;
}

VPOutPluginDummy::~VPOutPluginDummy()
{
    // make sure decoders are finished before calling
    ATOMIC_CAS(&work,true,false);
    resume();

    owner->mutex[0].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*bin->chans;i++)
        bin->buffer[i]=0.0f;
    owner->mutex[1].unlock();


    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
    free(out_buf);
    src_delete(rs);

}

