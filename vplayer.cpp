#include <thread>

#include "vplayer.h"
#include "outs/out_alsa.h"

void VPlayer::play_work(VPlayer *self)
{
    self->reader();
}

VPlayer::VPlayer()
{
    track_channels = 0;
    track_samples = 0;

    buffer1 = NULL;
    buffer2 = NULL;

    for (int i=0;i<4;i++){
        mutexes[i] = new std::mutex();
        mutexes[i]->lock();
    }

    play_worker = NULL;
    vpout=NULL;
    work=false;
}
void VPlayer::prepare()
{
    if (vpout != NULL){
        //stop();
        vpout->finit();

        for (int i=0;i<4;i++){
            mutexes[i]->lock();
        }
        delete buffer1;
        delete buffer2;
        delete vpout;
    }
    track_channels = 2;
    track_samples = 44100;

    buffer1 = new float[VPlayer::BUFFER_FRAMES*track_channels];
    buffer2 = new float[VPlayer::BUFFER_FRAMES*track_channels];

    vpout = (VPOutPlugin *) new VPOutPluginAlsa();
    vpout->setOwner(this);
    vpout->init(44100,2);
    paused = false;
}

int VPlayer::play()
{
    work = true;

    if (play_worker == NULL){
        play_worker = new std::thread(VPlayer::play_work,this);
    }
    vpout->resume();
    ((VPlayer *) this)->mutexes[0]->unlock();
    ((VPlayer *) this)->mutexes[2]->unlock();
}

void VPlayer::pause()
{vpout->pause();
    while (((VPlayer *) this)->mutexes[0]->try_lock() ||
           ((VPlayer *) this)->mutexes[1]->try_lock() ||
           ((VPlayer *) this)->mutexes[2]->try_lock() ||
           ((VPlayer *) this)->mutexes[3]->try_lock() ){ }
/*
    DBG(((VPlayer *) this)->mutexes[0]->try_lock());
    DBG(((VPlayer *) this)->mutexes[1]->try_lock());
    DBG(((VPlayer *) this)->mutexes[2]->try_lock());
    DBG(((VPlayer *) this)->mutexes[3]->try_lock());*/



    /*


    ((VPlayer *) this)->mutexes[0]->lock();
    ((VPlayer *) this)->mutexes[2]->lock();

    ((VPlayer *) this)->mutexes[1]->lock();

    ((VPlayer *) this)->mutexes[3]->lock();*/
}


