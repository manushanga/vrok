#ifndef PLAYER_MPEG_H
#define PLAYER_MPEG_H

#include <mpg123.h>

#include "../vplayer.h"

class MPEGPlayer : public VPlayer
{
public:
    mpg123_handle *mh;
    short *buffer;

    MPEGPlayer();
    int open(char *url);
    void reader();
    int setVolume(unsigned vol);
    unsigned long getLength();
    void setPosition(unsigned long t);
    unsigned long getPosition();
    ~MPEGPlayer();
};

#endif // PLAYER_MPEG_H
