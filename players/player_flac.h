#ifndef PLAYER_FLAC_H
#define PLAYER_FLAC_H

#include <FLAC/stream_decoder.h>

#include "../vplayer.h"

class FLACPlayer : public VPlayer
{
private:

public:
    bool work;
    float buffer[VPlayer::BUFFER_FRAMES*2*2];
    unsigned buffer_write;
    int open(char *url);
    int play();
    void pause();
    void stop();
    int setVolume(unsigned vol);
    unsigned long getLength();
    void setPosition(unsigned long t);
    unsigned long getPosition();
};

#endif // PLAYER_FLAC_H
