#ifndef PLAYER_AAC_H
#define PLAYER_AAC_H

#include <vorbis/vorbisfile.h>

#include "../vplayer.h"

class OGGPlayer : public VPlayer
{
public:
    OggVorbis_File vf;
    OGGPlayer();
    float *buffer;
    unsigned half_buffer_size;

    int open(const char *url);
    void reader();
    unsigned long getLength();
    void setPosition(unsigned long t);
    unsigned long getPosition();
    ~OGGPlayer();
};
#endif // PLAYER_AAC_H
