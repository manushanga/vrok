/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#include <unistd.h>

#include "vrok.h"
#include "mpeg.h"

#define SHORTTOFL (1.0f/__SHRT_MAX__)

VPDecoder* MPEGDecoder::VPDecoderMPEG_new(VPlayer *v)
{
    return (VPDecoder *)new MPEGDecoder(v);
}

MPEGDecoder::MPEGDecoder(VPlayer *v)
{
    int err;

    owner = v;
    mpg123_init();
    if ((mh = mpg123_new(NULL, &err)) == NULL){
        DBG("init fail");
    }
    buffer = NULL;

}

void MPEGDecoder::reader()
{
    int err = MPG123_OK;
    size_t done=0;
    size_t count = VPBUFFER_FRAMES*bout->chans*2;

    while (ATOMIC_CAS(&owner->work,true,true)) {
        while (done<count*sizeof(short) && err != MPG123_DONE){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
        }
        done=0;

        if (err != MPG123_OK && err != MPG123_DONE) {
            break;
        } else {
            owner->mutex[0].lock();
            for (size_t i=0;i<count/2;i++){
                bout->buffer1[i]=SHORTTOFL*buffer[i];
            }
            owner->postProcess(bout->buffer1);
            owner->mutex[1].unlock();

            owner->mutex[2].lock();
            for (size_t i=0;i<count/2;i++){
                bout->buffer2[i]=SHORTTOFL*buffer[count/2+i];
            }
            owner->postProcess(bout->buffer2);
            owner->mutex[3].unlock();
        }

        if (err == MPG123_DONE){
            break;
        }
    }
}

int MPEGDecoder::open(const char *url)
{
    fcurrent=fopenu(url,FOPEN_RB);
    if (mpg123_open_fd(mh, fileno(fcurrent)) != MPG123_OK) {
        DBG("open file fail");
        return -1;
    }
    int channels, encoding;
    long rate;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK){
        DBG("getformat fail");
        return -1;
    }

    buffer = new short[VPBUFFER_FRAMES*channels*2];

    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    if (encoding && MPG123_ENC_16 != MPG123_ENC_16) {
        DBG("unsupported");
        return -1;
    }

    DBG("meta done");
    VPBuffer bin;
    bin.srate = rate;
    bin.chans = channels;
    bin.buffer1 = NULL;
    bin.buffer2 = NULL;

    owner->setOutBuffers(&bin,&bout);
    for (unsigned i=0;i<VPBUFFER_FRAMES*bout->chans;i++){
        bout->buffer1[i]=0.0f;
        bout->buffer2[i]=0.0f;
    }
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
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    fclose(fcurrent);

    delete[] buffer;
}

