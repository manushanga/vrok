#include <stdlib.h>
#include <cstring>

#include "player_ogg.h"



OGGPlayer::OGGPlayer()
{
    reset();

}
int OGGPlayer::open(const char *url)
{
    int ret=ov_fopen(url,&vf);
    if (ret<0){
        DBG("OGGPlayer:open fail");
    }
    track_channels = ov_info(&vf,0)->channels;
    track_samplerate = ov_info(&vf,0)->rate;
    buffer = new float[VPlayer::BUFFER_FRAMES*track_channels*2];
    half_buffer_size = VPlayer::BUFFER_FRAMES*track_channels*sizeof(float);
    mutexes[0]->unlock();
    mutexes[2]->unlock();
}

void OGGPlayer::reader()
{
    float **pcm;
    long ret=1;
    int bit;
    size_t i,j,done=0;
    while (work && ret > 0 ){
        j=0;
        ret=1;
        done=0;
        while (done<VPlayer::BUFFER_FRAMES*2 && ret > 0){
            ret = ov_read_float( &vf, &pcm, VPlayer::BUFFER_FRAMES*2 - done,&bit );
            for (size_t i=0;i<ret;i++){
                for (size_t ch=0;ch<track_channels;ch++){
                    buffer[j]=pcm[ch][i]*volume;
                    j++;
                }

            }
            done+=ret;
        }

        if (ret == 0)
            break;
        mutexes[0]->lock();
        memcpy(buffer1,buffer,VPlayer::BUFFER_FRAMES*track_channels*sizeof(float) );
        post_process(buffer1);
        mutexes[1]->unlock();

        mutexes[2]->lock();
        memcpy(buffer2,buffer+VPlayer::BUFFER_FRAMES*track_channels ,VPlayer::BUFFER_FRAMES*track_channels*sizeof(float) );
        post_process(buffer2);
        mutexes[3]->unlock();

    }
}

unsigned long OGGPlayer::getLength()
{
    return ov_pcm_total(&vf,-1);
}

void OGGPlayer::setPosition(unsigned long t)
{
    if (ov_seekable(&vf))
        ov_pcm_seek(&vf,(ogg_int64_t)t);
}
unsigned long OGGPlayer::getPosition()
{
    return (unsigned long)ov_pcm_tell(&vf);
}
OGGPlayer::~OGGPlayer()
{
    ov_clear(&vf);
    delete buffer;
}
