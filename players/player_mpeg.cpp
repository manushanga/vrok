#include "player_mpeg.h"
#include "effect.h"

#define SHORTTOFL (1.0f/__SHRT_MAX__)

MPEGPlayer::MPEGPlayer()
{
    int err;
    mpg123_init();
    if ((mh = mpg123_new(NULL, &err)) == NULL){
        DBG("MPEGPlayer:MPEGPlayer init fail");
    }
    buffer = NULL;

}
void MPEGPlayer::reader()
{
    int err = MPG123_OK;
    size_t done=0;
    size_t count = ((VPlayer *)this)->BUFFER_FRAMES*((VPlayer *)this)->track_channels*2;

    while (work) {
        while (done<count*sizeof(short)){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
        }
        done=0;

        if (err != MPG123_OK) {
            break;
        } else {

            ((VPlayer *)this)->mutexes[0]->lock();
            for (size_t i=0;i<count/2;i++){
                ((VPlayer *)this)->buffer1[i]=SHORTTOFL*buffer[i];
            }
            ((VPlayer *)this)->vpeffect->process(buffer1);
            ((VPlayer *)this)->mutexes[1]->unlock();

            ((VPlayer *)this)->mutexes[2]->lock();
            for (size_t i=0;i<count/2;i++){
                ((VPlayer *)this)->buffer2[i]=SHORTTOFL*buffer[count/2+i];
            }
            ((VPlayer *)this)->vpeffect->process(buffer2);
            ((VPlayer *)this)->mutexes[3]->unlock();
        }


    }
}

int MPEGPlayer::open(char *url)
{
     if (mpg123_open(mh, url) != MPG123_OK) {
         DBG("MPEGPlayer:open open file fail");
         return -1;
     }
     int channels, encoding;
     long rate;
     if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK){
         DBG("MPEGPlayer:open getformat fail");
         return -1;
     }
     if (buffer!=NULL)
         delete buffer;
     buffer = new short[((VPlayer *)this)->BUFFER_FRAMES*((VPlayer *)this)->track_channels*2];
     ((VPlayer *) this)->mutexes[0]->unlock();
     ((VPlayer *) this)->mutexes[2]->unlock();
     DBG("asd");
     mpg123_format_none(mh);
     mpg123_format(mh, rate, channels, encoding);
}

int MPEGPlayer::setVolume(unsigned vol)
{
    return 0;
}
unsigned long MPEGPlayer::getLength()
{
    return 0;
}
void MPEGPlayer::setPosition(unsigned long t)
{

}
unsigned long MPEGPlayer::getPosition()
{
    return 0;
}

