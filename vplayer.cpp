#include <thread>

#include "vplayer.h"
#include "outs/out_alsa.h"

void VPlayer::init()
{

    buffer1 = new float[VPlayer::BUFFER_FRAMES*2];
    buffer2 = new float[VPlayer::BUFFER_FRAMES*2];
    track_channels = 2;
    track_samples = 44100;

    for (int i=0;i<4;i++){
        mutexes[i] = new std::mutex();
        mutexes[i]->lock();
    }

    vpout = (VPOutPlugin *) new VPOutPluginAlsa();
    vpout->setOwner(this);
    vpout->init(44100,2);
}

void VPlayer::end()
{
    stop();
    vpout->end();
}

