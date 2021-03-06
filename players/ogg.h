#ifndef PLAYER_AAC_H
#define PLAYER_AAC_H

#include <vorbis/vorbisfile.h>

#include "vplayer.h"
#include "decoder.h"
class OGGDecoder : public VPDecoderPlugin
{
private:
    FILE *fcurrent;
public:
    static VPDecoderPlugin* VPDecoderOGG_new(VPlayer *v);
    OggVorbis_File vf;
    float *buffer;
    unsigned half_buffer_size;
    uint64_t seek_to;

    OGGDecoder(VPlayer *v);
    int open(VPResource resource);
    void reader();
    uint64_t getLength();
    void setPosition(uint64_t t);
    uint64_t getPosition();
    ~OGGDecoder();
};

#endif // PLAYER_AAC_H
