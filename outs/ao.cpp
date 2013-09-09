/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <unistd.h>
#include "vrok.h"
#include "ao.h"

VPOutPlugin* VPOutPluginAO::VPOutPluginAO_new()
{
    return (VPOutPlugin *) new VPOutPluginAO();
}

void VPOutPluginAO::worker_run(VPOutPluginAO *self)
{
    int error;
    unsigned chans=self->bin->chans;

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

        self->owner->mutex[1].lock();

        for (int i=0;i<VPBUFFER_FRAMES*chans;i++){
            self->buffer[i]=(unsigned short)(self->bin->buffer[1-*self->bin->cursor][i]*32767.0f);
		}
        ao_play(self->device, (char *)self->buffer,VPBUFFER_FRAMES*chans*sizeof(unsigned short));

        self->owner->mutex[0].unlock();
    }

}

void __attribute__((optimize("O0"))) VPOutPluginAO::rewind()
{

    if (m_pause.try_lock()){
        //m_pause.lock();
        ATOMIC_CAS(&pause_check,false,true);

        owner->mutex[0].lock();
        for (unsigned i=0;i<VPBUFFER_FRAMES*bin->chans;i++)
            bin->buffer[*bin->cursor][i]=0.0f;
        owner->mutex[1].unlock();

        while (!ATOMIC_CAS(&paused,false,false)) {}
    }

}

void __attribute__((optimize("O0"))) VPOutPluginAO::resume()
{
    if (ATOMIC_CAS(&paused,true,true)){
        ATOMIC_CAS(&pause_check,true,false);

        m_pause.try_lock();
        m_pause.unlock();
        while (ATOMIC_CAS(&paused,true,true)) {}
    }
}
void __attribute__((optimize("O0"))) VPOutPluginAO::pause()
{
    if (!ATOMIC_CAS(&paused,false,false)){

        if (m_pause.try_lock()){
            ATOMIC_CAS(&pause_check,false,true);
            while (!ATOMIC_CAS(&paused,false,false)) {}
        }
        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

int VPOutPluginAO::init(VPlayer *v, VPBuffer *in)
{
	ao_sample_format format;
	int default_driver;
	
    DBG("AO:init");
    owner = v;
    bin=in;

	ao_initialize();
	default_driver = ao_default_driver_id();
	memset(&format, 0, sizeof(format));
	format.bits = 16;
    format.channels = in->chans;
	format.rate = in->srate;
    format.byte_format = AO_FMT_LITTLE;
	
	/* -- Open driver -- */
	device = ao_open_live(default_driver, &format, NULL /* no options */);
	if (device == NULL) {
		DBG("Error opening device");
		return 1;
	}
    buffer= new unsigned short[VPBUFFER_FRAMES*in->chans];
    work = true;
    paused = false;
    pause_check = false;

    worker = new std::thread((void(*)(void*))worker_run, this);
    DBG("ao thread made");
    return 0;
}

VPOutPluginAO::~VPOutPluginAO()
{
    rewind();
    ATOMIC_CAS(&work,true,false);
    resume();


    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
	
	delete[] buffer;
	
    ao_close(device);

	ao_shutdown();

}
