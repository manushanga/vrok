/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <unistd.h>

#include "vrok.h"
#include "pulse.h"

VPOutPlugin* VPOutPluginPulse::VPOutPluginPulse_new()
{
    return (VPOutPlugin *) new VPOutPluginPulse();
}

void VPOutPluginPulse::worker_run(VPOutPluginPulse *self)
{
    int ret,error;
    unsigned chans=self->owner->track_channels;

    while (ATOMIC_CAS(&self->work,true,true)){
        if (ATOMIC_CAS(&self->pause_check,true,true)) {
            //pa_simple_drain(self->handle,&error);
           // pa_simple_flush(self->handle,&error);
            ATOMIC_CAS(&self->paused,false,true);
            self->m_pause.lock();
            self->m_pause.unlock();
            ATOMIC_CAS(&self->paused,true,false);
            ATOMIC_CAS(&self->pause_check,true,false);
        }

        self->owner->mutexes[1].lock();

        pa_simple_write(self->handle,self->owner->buffer1,VPBUFFER_FRAMES*sizeof(float)*chans,&error);

        self->owner->mutexes[0].unlock();

        self->owner->mutexes[3].lock();

        pa_simple_write(self->handle,self->owner->buffer2,VPBUFFER_FRAMES*sizeof(float)*chans,&error);

        self->owner->mutexes[2].unlock();
    }

}

void __attribute__((optimize("O0"))) VPOutPluginPulse::rewind()
{

    owner->mutexes[0].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer1[i]=0.0f;
    owner->mutexes[1].unlock();
    owner->mutexes[2].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer2[i]=0.0f;
    owner->mutexes[3].unlock();

    m_pause.lock();
    ATOMIC_CAS(&pause_check,false,true);

    while (!ATOMIC_CAS(&paused,false,false)) {}

}

void __attribute__((optimize("O0"))) VPOutPluginPulse::resume()
{
    if (ATOMIC_CAS(&paused,true,true)){
        ATOMIC_CAS(&pause_check,true,false);

        m_pause.try_lock();
        m_pause.unlock();
        while (ATOMIC_CAS(&paused,true,true)) {}
    }
}
void __attribute__((optimize("O0"))) VPOutPluginPulse::pause()
{
    if (!ATOMIC_CAS(&paused,false,false)){

        m_pause.lock();

        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

int VPOutPluginPulse::init(VPlayer *v, unsigned samplerate, unsigned channels)
{
    DBG("Pulse:init");
    owner = v;
    int error;
    pa_sample_spec ss;
    ss.channels = channels;
    ss.rate = samplerate;
    ss.format = PA_SAMPLE_FLOAT32LE;
    handle = pa_simple_new(NULL,"Vrok",PA_STREAM_PLAYBACK,NULL,"Music",(const pa_sample_spec *)&ss,NULL,NULL,&error);
    if (!handle){
        DBG("Pulse:init: failed to open pcm");
        return -1;
    }

    work = true;
    paused = false;
    pause_check = false;

    worker = new std::thread((void(*)(void*))worker_run, this);
    DBG("pulse thread made");
    return 0;
}

VPOutPluginPulse::~VPOutPluginPulse()
{
    // make sure decoders are finished before calling
    ATOMIC_CAS(&work,true,false);
    resume();

    owner->mutexes[0].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer1[i]=0.0f;
    owner->mutexes[1].unlock();

    owner->mutexes[2].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer2[i]=0.0f;
    owner->mutexes[3].unlock();

    pa_simple_free(handle);

    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }

}
