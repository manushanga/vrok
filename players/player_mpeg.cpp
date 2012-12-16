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

        if (stopped)
            mutex_pause->lock();

        while (done<count*sizeof(short) && err != MPG123_DONE){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
        }
        done=0;

        if (err != MPG123_OK && err != MPG123_DONE) {
            break;
        } else {

            mutexes[0]->lock();
            for (size_t i=0;i<count/2;i++){
                buffer1[i]=SHORTTOFL*buffer[i];
            }
            post_process(buffer1);
            mutexes[1]->unlock();

            mutexes[2]->lock();
            for (size_t i=0;i<count/2;i++){
                buffer2[i]=SHORTTOFL*buffer[count/2+i];
            }
            post_process(buffer2);
            mutexes[3]->unlock();
        }

        if (err == MPG123_DONE){
            break;
        }
    }
    // stub for handling track end
    if (work){
        ended();
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
MPEGPlayer::~MPEGPlayer()
{
    this->~VPlayer();
}

