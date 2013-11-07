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
    virtual uint64_t getLength() = 0;
    virtual void setPosition(uint64_t t) = 0;
    virtual uint64_t getPosition() = 0;
    virtual ~VPDecoderPlugin() {}
};


#endif // DECODER_H
