#ifndef OUT_H
#define OUT_H

#include <stdint.h>
#include <thread>

#include "vputils.h"
#include "vplayer.h"

class VPOutPlugin{
public:
    VPlayer *owner;
    bool work;
    void setOwner(VPlayer *v);
    virtual const char *getName()=0;
    virtual int init(unsigned samplerate, unsigned channels)=0;
    virtual unsigned getSamplerate()=0;
    virtual unsigned getChannels()=0;
    virtual int end()=0;
};


#endif // OUT_H
