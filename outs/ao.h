#ifndef AO_H
#define AO_H

#include <ao/ao.h>

#include "vplayer.h"
#include "out.h"


class VPOutPluginAO : public VPOutPlugin {
public:
    static VPOutPlugin* VPOutPluginAO_new();
    static void worker_run(VPOutPluginAO *self);
    VPlayer *owner;
    ao_device *device;
	unsigned short *buffer;
    std::thread *worker;
    std::mutex m_pause;
    volatile bool work;
    volatile bool pause_check;
    volatile bool paused;

    virtual int init(VPlayer *v, VPBuffer *in);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual ~VPOutPluginAO();
};

#endif // AO_H
