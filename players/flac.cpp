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
        me->to_fl = 1.0f/pow(2,metadata->data.stream_info.bits_per_sample);
        me->buffer_bytes = VPBUFFER_FRAMES*metadata->data.stream_info.channels*sizeof(float);
        me->buffer = new float[VPBUFFER_FRAMES*metadata->data.stream_info.channels];

        VPBuffer bin;
        bin.srate = metadata->data.stream_info.sample_rate;
        bin.chans = metadata->data.stream_info.channels;
        bin.buffer[0] = NULL;
        bin.buffer[1] = NULL;
        me->owner->setOutBuffers(&bin,&me->bout);


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

    // general overview
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

    if (selfp->buffer_write+frame->header.blocksize*selfp->bout->chans + 1 < VPBUFFER_FRAMES*selfp->bout->chans){
        size_t i=0,j=0;
        //DBG("s1");
        while (i<frame->header.blocksize) {
            for (unsigned ch=0;ch<selfp->bout->chans;ch++){
                selfp->buffer[selfp->buffer_write+j]=selfp->to_fl*buffer[ch][i];
                j++;
            }
            i++;
        }
        selfp->buffer_write+=j;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    } else if (selfp->buffer_write+frame->header.blocksize*selfp->bout->chans + 1 == VPBUFFER_FRAMES*selfp->bout->chans) {
        size_t i=0,j=0;

        while (i<frame->header.blocksize) {
            for (unsigned ch=0;ch<selfp->bout->chans;ch++){
                selfp->buffer[selfp->buffer_write+j]=selfp->to_fl*buffer[ch][i];
                j++;
            }
            i++;
        }
        j=0;

        // write buffer
        memcpy(selfp->bout->buffer[*selfp->bout->cursor],selfp->buffer,selfp->buffer_bytes );
        self->postProcess(selfp->bout->buffer[*selfp->bout->cursor]);

        self->mutex[0].lock();
        VP_SWAP_BUFFERS(selfp->bout);
        self->mutex[1].unlock();


        selfp->buffer_write=0;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    } else {

        size_t i=0,j=selfp->buffer_write;

        while (j<VPBUFFER_FRAMES*selfp->bout->chans) {
            for (unsigned ch=0;ch<selfp->bout->chans;ch++){
                selfp->buffer[j]=selfp->to_fl*buffer[ch][i];
                j++;
            }
            i++;
        }

        j=0;
        // write buffer
        memcpy(selfp->bout->buffer[*selfp->bout->cursor],selfp->buffer,selfp->buffer_bytes);

        self->postProcess(selfp->bout->buffer[*selfp->bout->cursor]);


        self->mutex[0].lock();
        VP_SWAP_BUFFERS(selfp->bout);
        self->mutex[1].unlock();


        if (!ATOMIC_CAS(&self->work,false,false))
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

        while (frame->header.blocksize-i > VPBUFFER_FRAMES ){

            j=0;
            // write buffer
            while(j<VPBUFFER_FRAMES*selfp->bout->chans){
                for (unsigned ch=0;ch<selfp->bout->chans;ch++){
                    selfp->bout->buffer[*selfp->bout->cursor][j]=selfp->to_fl*buffer[ch][i];
                    j++;
                }
                i++;
            }
            self->postProcess(selfp->bout->buffer[*selfp->bout->cursor]);


            self->mutex[0].lock();
            VP_SWAP_BUFFERS(selfp->bout);
            self->mutex[1].unlock();

            if (!ATOMIC_CAS(&self->work,false,false))
                return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }

        j=0;
        while (i<frame->header.blocksize) {
            for (unsigned ch=0;ch<selfp->bout->chans;ch++){
                selfp->buffer[j]=selfp->to_fl*buffer[ch][i];
                j++;
            }
            i++;
        }
        selfp->buffer_write=j;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
}

FLACDecoder::FLACDecoder(VPlayer *v)
{
    buffer = NULL;
    decoder = NULL;
    owner = v;
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
    delete[] buffer;
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
    } else {
        DBG("Error "<<FLAC__StreamDecoderInitStatusString[init_status]);
    }

}

unsigned long FLACDecoder::getLength()
{
    return (unsigned long)FLAC__stream_decoder_get_total_samples(decoder);
}

void FLACDecoder::setPosition(unsigned long t)
{
    // worst seeking evaar!
    FLAC__stream_decoder_skip_single_frame(decoder);
    FLAC__stream_decoder_seek_absolute(decoder, (uint64_t)t);

}
unsigned long FLACDecoder::getPosition()
{
    uint64_t pos=0;
    FLAC__stream_decoder_get_decode_position(decoder, &pos);
    return (unsigned long) pos;
}
