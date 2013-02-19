/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#include <unistd.h>

#include "config_out.h"
#include "mpeg.h"

#define SHORTTOFL (1.0f/__SHRT_MAX__)

VPDecoder *_VPDecoderMPEG_new()
{
    return (VPDecoder *)new MPEGDecoder();
}

MPEGDecoder::MPEGDecoder()
{
    int err;
    mpg123_init();
    if ((mh = mpg123_new(NULL, &err)) == NULL){
        DBG("init fail");
    }
    buffer = NULL;

}
void MPEGDecoder::init(VPlayer *v)
{
    owner = v;
}
void MPEGDecoder::reader()
{
    int err = MPG123_OK;
    size_t done=0;
    size_t count = VPBUFFER_FRAMES*owner->track_channels*2;

    while (owner->work) {
        while (done<count*sizeof(short) && err != MPG123_DONE){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
        }
        done=0;

        if (err != MPG123_OK && err != MPG123_DONE) {
            break;
        } else {

            owner->mutexes[0].lock();
            for (size_t i=0;i<count/2;i++){
                owner->buffer1[i]=SHORTTOFL*buffer[i]*owner->volume;
            }
            owner->post_process(owner->buffer1);
            owner->mutexes[1].unlock();

            owner->mutexes[2].lock();
            for (size_t i=0;i<count/2;i++){
                owner->buffer2[i]=SHORTTOFL*buffer[count/2+i]*owner->volume;
            }
            owner->post_process(owner->buffer2);
            owner->mutexes[3].unlock();
        }

        if (err == MPG123_DONE){
            break;
        }
    }

    if (owner->work && err == MPG123_DONE){
        owner->ended();
    }
}

int MPEGDecoder::open(const char *url)
{

    if (mpg123_open(mh, url) != MPG123_OK) {
        DBG("open file fail");
        return -1;
    }
    int channels, encoding;
    long rate;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK){
        DBG("getformat fail");
        return -1;
    }

    owner->set_metadata( rate,  channels);

    if (buffer!=NULL)
        delete buffer;
    buffer = new short[VPBUFFER_FRAMES*owner->track_channels*2];

    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    DBG("meta done");
    owner->vpout_open();
    return 0;
}

unsigned long MPEGDecoder::getLength()
{
    mpg123_seek(mh, (off_t) 0, SEEK_END);
    off_t pos=mpg123_tell(mh);
    return (unsigned long)pos;
}
void MPEGDecoder::setPosition(unsigned long t)
{
    mpg123_seek(mh, (off_t) t, SEEK_SET);
}
unsigned long MPEGDecoder::getPosition()
{
    return (unsigned long)mpg123_tell(mh);
}
MPEGDecoder::~MPEGDecoder()
{
    owner->vpout_close();
    mpg123_close(mh);
    mpg123_exit();
    if (buffer)
        delete buffer;
}

