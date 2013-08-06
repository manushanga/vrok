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
    int err = MPG123_ERR;
    size_t done=0;
    size_t count = VPBUFFER_FRAMES*bout->chans;

    while (ATOMIC_CAS(&owner->work,true,true)) {
        while (done<count*sizeof(short) && err != MPG123_DONE){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
        }
        done=0;

        if (err == MPG123_ERR) {
            DBG("MPEG error");
            break;
        } else {
            for (size_t i=0;i<count;i++){
                bout->buffer[*bout->cursor][i]=SHORTTOFL*buffer[i];
            }
            owner->postProcess(bout->buffer[*bout->cursor]);
            owner->mutex[0].lock();
            VP_SWAP_BUFFERS(bout);
            owner->mutex[1].unlock();
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

    buffer = new short[VPBUFFER_FRAMES*channels];

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
    bin.buffer[0] = NULL;
    bin.buffer[1] = NULL;

    owner->setOutBuffers(&bin,&bout);
    for (unsigned i=0;i<VPBUFFER_FRAMES*bout->chans;i++){
        bout->buffer[0][i]=0.0f;
        bout->buffer[1][i]=0.0f;

    }
    DBG("mpeg open done");
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

