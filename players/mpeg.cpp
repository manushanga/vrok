/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifdef __linux__
#include <unistd.h>
#endif
#include "vrok.h"
#include "mpeg.h"

#define SHORTTOFL (1.0f/32768.0f)
#define SEEK_MAX 0xFFFFFFFFFFFFFFFFL

VPDecoderPlugin* MPEGDecoder::VPDecoderMPEG_new(VPlayer *v)
{
    return (VPDecoderPlugin *)new MPEGDecoder(v);
}

MPEGDecoder::MPEGDecoder(VPlayer *v) : seek_to(SEEK_MAX) ,buffer(NULL)
{
    int err;
    owner = v;
    mpg123_init();
    if ((mh = mpg123_new(NULL, &err)) == NULL){
        DBG("init fail");
    }
}

void MPEGDecoder::reader()
{
    int err = MPG123_ERR;
    size_t done=0;
    size_t totalDone=0;
    DBG(&owner->bout<<bout);
    size_t count = VPBUFFER_FRAMES*bout->chans;

    while (ATOMIC_CAS(&owner->work,true,true)) {

        if (ATOMIC_CAS(&seek_to,SEEK_MAX,SEEK_MAX) != SEEK_MAX ){
            mpg123_seek(mh, (off_t) seek_to,SEEK_SET);
            seek_to = SEEK_MAX;
        }
        totalDone=0;
        while (done<count*sizeof(short) && err != MPG123_DONE){
            err = mpg123_read( mh, ((unsigned char *) buffer)+done, count*sizeof(short)-done, &done );
            totalDone+=done;
        }
        *bout->currentBufferSamples() =  totalDone/(bout->chans*sizeof(short));
        done=0;

        if (err == MPG123_ERR) {
            DBG("MPEG error");
            break;
        } else {
            for (size_t i=0;i<count;i++){
                bout->currentBuffer()[i]=SHORTTOFL*buffer[i];
            }
            owner->postProcess();

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

    buffer =(short *) ALIGNED_ALLOC(sizeof(short)*VPBUFFER_FRAMES*channels);

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

uint64_t MPEGDecoder::getLength()
{
    return (uint64_t)mpg123_length(mh);
}
void MPEGDecoder::setPosition(uint64_t t)
{
    if ( ATOMIC_CAS(&seek_to,SEEK_MAX,SEEK_MAX) == SEEK_MAX){
        seek_to = t;
    }

}
uint64_t MPEGDecoder::getPosition()
{
    return (uint64_t)mpg123_tell(mh);
}
MPEGDecoder::~MPEGDecoder()
{
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    fclose(fcurrent);

    ALIGNED_FREE(buffer);
}

