#ifndef DECODER_H
#define DECODER_H

#include "vplayer.h"

class VPDecoderPlugin
{
protected:
    VPBuffer *bout;
public:
    VPlayer *owner;
    virtual void reader() = 0;
    virtual int open(const char *url) = 0;
    virtual unsigned long getLength() = 0;
    virtual void setPosition(unsigned long t) = 0;
    virtual unsigned long getPosition() = 0;
    virtual ~VPDecoderPlugin() {}
};


#endif // DECODER_H
