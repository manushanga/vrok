#include <stdlib.h>
#include <cstring>

#include "ogg.h"

VPDecoder *_VPDecoderOgg_new()
{
    return (VPDecoder *)new OGGDecoder();
}

OGGDecoder::OGGDecoder()
{

}
void OGGDecoder::init(VPlayer *v)
{
    owner = v;
}
int OGGDecoder::open(const char *url)
{

    int ret=ov_fopen(url,&vf);
    if (ret<0){
        DBG("OGGPlayer:open fail");
        return -1;
    }

    owner->set_metadata(ov_info(&vf,0)->rate, ov_info(&vf,0)->channels);
    buffer = new float[VPlayer::BUFFER_FRAMES*owner->track_channels*2];
    half_buffer_size = VPlayer::BUFFER_FRAMES*owner->track_channels*sizeof(float);

    DBG("OGG Vorbis meta done");
    owner->vpout_open();
    return 0;
}

void OGGDecoder::reader()
{
    float **pcm;
    long ret=1;
    int bit;
    size_t i,j,done=0;
    while (owner->work && ret > 0 ){
        j=0;
        ret=1;
        done=0;
        while (done<VPlayer::BUFFER_FRAMES*2 && ret > 0){
            ret = ov_read_float( &vf, &pcm, VPlayer::BUFFER_FRAMES*2 - done,&bit );
            for (size_t i=0;i<ret;i++){
                for (size_t ch=0;ch<owner->track_channels;ch++){
                    buffer[j]=pcm[ch][i]*owner->volume;
                    j++;
                }
            }
            done+=ret;
        }

        if (ret == 0)
            break;
        owner->mutexes[0].lock();
        memcpy(owner->buffer1,buffer,VPlayer::BUFFER_FRAMES*owner->track_channels*sizeof(float) );
        owner->post_process(owner->buffer1);
        owner->mutexes[1].unlock();

        owner->mutexes[2].lock();
        memcpy(owner->buffer2,buffer+VPlayer::BUFFER_FRAMES*owner->track_channels ,VPlayer::BUFFER_FRAMES*owner->track_channels*sizeof(float) );
        owner->post_process(owner->buffer2);
        owner->mutexes[3].unlock();

    }
    if (owner->work && ret > 0){
        owner->ended();
    }
}

unsigned long OGGDecoder::getLength()
{
    return ov_pcm_total(&vf,-1);
}

void OGGDecoder::setPosition(unsigned long t)
{
    if (ov_seekable(&vf))
        ov_pcm_seek(&vf,(ogg_int64_t)t);
}
unsigned long OGGDecoder::getPosition()
{
    return (unsigned long)ov_pcm_tell(&vf);
}
OGGDecoder::~OGGDecoder()
{
    owner->vpout_close();
    ov_clear(&vf);
    delete buffer;
}
