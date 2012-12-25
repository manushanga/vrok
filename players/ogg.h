#ifndef PLAYER_AAC_H
#define PLAYER_AAC_H

#include <vorbis/vorbisfile.h>

#include "../vplayer.h"
#include "../decoder.h"
class OGGDecoder : public VPDecoder
{
public:
    OggVorbis_File vf;
    OGGDecoder();
    float *buffer;
    unsigned half_buffer_size;
    VPlayer *owner;

    void init(VPlayer *v);
    int open(const char *url);
    void reader();
    unsigned long getLength();
    void setPosition(unsigned long t);
    unsigned long getPosition();
    ~OGGDecoder();
};

VPDecoder* _VPDecoderOgg_new();

#endif // PLAYER_AAC_H
