/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include <cmath>

#include "vrok.h"
#include "flac.h"

#define SEEK_MAX 0xFFFFFFFFFFFFFFFFL

VPDecoderPlugin* FLACDecoder::VPDecoderFLAC_new(VPlayer *v)
{
    return (VPDecoderPlugin *)new FLACDecoder(v);
}
void FLACDecoder::metadata_callback(const FLAC__StreamDecoder *decoder,
                              const FLAC__StreamMetadata *metadata,
                              void *client_data)
{
    FLACDecoder *me = (FLACDecoder*) client_data;

    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        me->to_fl = 2.0f/pow(2.0,metadata->data.stream_info.bits_per_sample*1.0);

        me->buffer_bytes = VPBUFFER_FRAMES*metadata->data.stream_info.channels*sizeof(float);
        me->buffer = (float *) ALIGNED_ALLOC(sizeof(float)*VPBUFFER_FRAMES*metadata->data.stream_info.channels);

        VPBuffer bin;
        bin.srate = metadata->data.stream_info.sample_rate;
        bin.chans = metadata->data.stream_info.channels;
        bin.buffer[0] = NULL;
        bin.buffer[1] = NULL;
        me->owner->setOutBuffers(&bin,&me->bout);
        me->bps = metadata->data.stream_info.bits_per_sample;

        for (unsigned i=0;i<VPBUFFER_FRAMES*me->bout->chans;i++){
            me->bout->buffer[0][i]=0.0f;
            me->bout->buffer[1][i]=0.0f;

        }
    }

    DBG("meta done");

}

static void error_callback(const FLAC__StreamDecoder *decoder,
                           FLAC__StreamDecoderErrorStatus status,
                           void *client_data)
{
    FLACDecoder *me = (FLACDecoder*) client_data;
    DBG(FLAC__StreamDecoderErrorStatusString[status]);
    ATOMIC_CAS(&me->owner->work,true,false);
}

FLAC__StreamDecoderWriteStatus FLACDecoder::write_callback(const FLAC__StreamDecoder *decoder,
                                                     const FLAC__Frame *frame,
                                                     const FLAC__int32 * const buffer[],
                                                     void *client_data)
{
    VPlayer *self = ((FLACDecoder*) client_data)->owner;
    FLACDecoder *selfp = (FLACDecoder*) client_data;


    float to_fl=selfp->to_fl;
    // general overview
    // seek if requested
    // fill buffer if not full
    // return ok
    // if full
    // write buffer
    // write buffer2
    // if blocksize<buffersize
    //  fill
    //  return ok
    // else
    //  write full buffers to buffer buffer2
    //  write remaining to buffer
    //  return ok


    if (!ATOMIC_CAS(&self->work,false,false))
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    if (ATOMIC_CAS(&selfp->seek_to,SEEK_MAX,SEEK_MAX) != SEEK_MAX ){
        uint64_t p= selfp->seek_to;
        selfp->seek_to =SEEK_MAX;
        FLAC__stream_decoder_seek_absolute(selfp->decoder, (FLAC__uint64)p);

        selfp->buffer_write = 0;
        //return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }


    size_t i=0,j=0;

    while (i<frame->header.blocksize) {

        for (unsigned ch=0;ch<selfp->bout->chans;ch++){
            selfp->bout->currentBuffer()[selfp->buffer_write]=to_fl*buffer[ch][i];
            selfp->buffer_write++;
        }
        i++;
        if (selfp->buffer_write >= VPBUFFER_FRAMES * selfp->bout->chans) {
            *selfp->bout->currentBufferSamples()  =  selfp->buffer_write/selfp->bout->chans;
            self->postProcess();

            self->mutex[0].lock();
            VP_SWAP_BUFFERS(selfp->bout);
            self->mutex[1].unlock();

            selfp->buffer_write = 0;
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLACDecoder::FLACDecoder(VPlayer *v) : seek_to(SEEK_MAX), buffer(NULL), decoder(NULL) , ret_vpout_open(0)
{
    owner=v;
    if ((decoder = FLAC__stream_decoder_new()) == NULL) {
        DBG("FLACPlayer:open: decoder create fail");
    }
    buffer_write = 0;
    init_status = FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE;
}

FLACDecoder::~FLACDecoder()
{
    FLAC__stream_decoder_finish(decoder);
    FLAC__stream_decoder_delete(decoder);
}

int FLACDecoder::open(const char *url)
{
    fcurrent = fopenu(url,FOPEN_RB);
    init_status = FLAC__stream_decoder_init_FILE(decoder, fcurrent, write_callback, metadata_callback, error_callback, (void *) this);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK){
        DBG("decoder init fail");
        return -1;
    } else {
        FLAC__stream_decoder_process_until_end_of_metadata(decoder);
    }
    return ret_vpout_open;
}

void FLACDecoder::reader()
{
    if (init_status==FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        FLAC__stream_decoder_process_until_end_of_stream(decoder);
        if (buffer_write > 0) {

            *bout->currentBufferSamples() = buffer_write / bout->chans;
            owner->postProcess();
#ifdef SINGLE_CORE
            owner->vpout->writeSingleCore();
#else
            owner->mutex[0].lock();
            VP_SWAP_BUFFERS(bout);
            owner->mutex[1].unlock();
#endif
        }
    } else {
        DBG("Error "<<FLAC__StreamDecoderInitStatusString[init_status]);
    }

}

uint64_t FLACDecoder::getLength()
{// TODO: check if decoder is started
    if (ATOMIC_CAS(&owner->work,true,true))
        return (uint64_t)FLAC__stream_decoder_get_total_samples(decoder);
    else
        return 0;
}

void FLACDecoder::setPosition(uint64_t t)
{
    if ( ATOMIC_CAS(&owner->work,true,true) && ATOMIC_CAS(&seek_to,SEEK_MAX,SEEK_MAX) == SEEK_MAX){
        seek_to = t;
    }
}
uint64_t FLACDecoder::getPosition()
{
    if (ATOMIC_CAS(&owner->work,true,true)) {
        uint64_t pos=0;
        FLAC__stream_decoder_get_decode_position(decoder, &pos);
        DBG(pos);
        return (uint64_t) ((pos*8) / bps);
    } else {
        return 0;
    }
}
