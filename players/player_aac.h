#ifndef PLAYER_AAC_H
#define PLAYER_AAC_H
#include <stdlib.h>
#include <neaacdec.h>
#include "../vplayer.h"

class AACPlayer : public VPlayer
{
public:
    NeAACDecHandle decoder;
    FILE *f;
    size_t read_head;

    AACPlayer();
    int open(const char *url);
    void reader();
    unsigned long getLength();
    void setPosition(unsigned long t);
    unsigned long getPosition();
    ~AACPlayer();
};
#endif // PLAYER_AAC_H
