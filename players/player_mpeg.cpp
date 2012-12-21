/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

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
        while (done<count*sizeof(short) && err != MPG123_DONE){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
        }
        done=0;

        if (err != MPG123_OK && err != MPG123_DONE) {
            break;
        } else {

            mutexes[0]->lock();
            for (size_t i=0;i<count/2;i++){
                buffer1[i]=SHORTTOFL*buffer[i]*volume;
            }
            post_process(buffer1);
            mutexes[1]->unlock();

            mutexes[2]->lock();
            for (size_t i=0;i<count/2;i++){
                buffer2[i]=SHORTTOFL*buffer[count/2+i]*volume;
            }
            post_process(buffer2);
            mutexes[3]->unlock();
        }

        if (err == MPG123_DONE){
            break;
        }
    }

    if (work && err == MPG123_DONE){
        ended();
    }
}

int MPEGPlayer::open(const char *url)
{
    mutex_control->lock();

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

    set_metadata( rate,  channels);


    if (buffer!=NULL)
        delete buffer;
    buffer = new short[((VPlayer *)this)->BUFFER_FRAMES*((VPlayer *)this)->track_channels*2];

    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    mutex_control->unlock();

    vpout_open();
    return 0;
}

unsigned long MPEGPlayer::getLength()
{
    mpg123_seek(mh, (off_t) 0, SEEK_END);
    off_t pos=mpg123_tell(mh);
    return (unsigned long)pos;
}
void MPEGPlayer::setPosition(unsigned long t)
{
    mpg123_seek(mh, (off_t) t, SEEK_SET);
}
unsigned long MPEGPlayer::getPosition()
{
    return (unsigned long)mpg123_tell(mh);
}
MPEGPlayer::~MPEGPlayer()
{
    vpout_close();
    if (buffer)
        delete buffer;
}

