#include <stdlib.h>
#include <cstring>

#include "player_aac.h"

// Vrok doesn't have a channel limit, but have to come to a
// conclusion because of FAAD2 design
#define BUFFER_SIZE FAAD_MIN_STREAMSIZE*9

AACPlayer::AACPlayer()
{
    if ( !(FIXED_POINT_CAP && NeAACDecGetCapabilities()) ) {
        DBG("AACPlayer:AACPlayer the decoder you have can't decode to float");
    }
    decoder = NeAACDecOpen();
    NeAACDecConfiguration *p = NeAACDecGetCurrentConfiguration(decoder);
    p->outputFormat = FAAD_FMT_FLOAT;
    p->downMatrix = 0;
    NeAACDecSetConfiguration(decoder, p);

}
int AACPlayer::open(const char *url)
{
    f=fopen(url,"r");
    unsigned char head[BUFFER_SIZE];
    size_t r = fread(head,BUFFER_SIZE,1,f);
    unsigned long s;
    unsigned char c;
    long read = NeAACDecInit(decoder,head,r,&s,&c);
    if (read < 0){
        DBG("AACPlayer:open fail");
        return -1;
    }
    track_samplerate = (unsigned) s;
    track_channels = (unsigned) c;
    fseek(f,read,SEEK_SET);

    vpout_open();
    return 0;
}

void AACPlayer::reader()
{
    unsigned char buffer[BUFFER_SIZE];

    size_t read, fpos=(size_t)ftell(f), write;
    NeAACDecFrameInfo hfinfo;
    memset(&hfinfo,0,sizeof(NeAACDecFrameInfo));
    while (!feof(f)){
        read = fread(buffer,BUFFER_SIZE,1,f);
        float *samples=NeAACDecDecode(decoder,hfinfo,buffer,read);
        write=hfinfo.samples;
        while (write>)
        fpos += hfinfo.bytesconsumed;
        fseek(f,fpos,SEEK_SET);


    }
}

unsigned long AACPlayer::getLength()
{
    return 0;
}

void AACPlayer::setPosition(unsigned long t)
{

}
unsigned long AACPlayer::getPosition()
{
    return 0;
}
AACPlayer::~AACPlayer()
{

}
