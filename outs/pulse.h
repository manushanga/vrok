#ifndef PULSE_H
#define PULSE_H

#include <pulse/simple.h>

#include "vplayer.h"
#include "out.h"


class VPOutPluginPulse : public VPOutPlugin {
public:
    static VPOutPlugin* VPOutPluginPulse_new();
    static void worker_run(VPOutPluginPulse *self);
    VPlayer *owner;
    pa_simple *handle;
    std::thread *worker;
    std::mutex m_pause;
    volatile bool work;
    volatile bool pause_check;
    volatile bool paused;

    virtual int init(VPlayer *v, VPBuffer *in);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual ~VPOutPluginPulse();
};

#endif // PULSE_H
