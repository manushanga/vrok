#include <thread>

#include "vplayer.h"
#include "outs/out_alsa.h"
#include "effects/effect_eq.h"

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
        vpout->finit();
        vpeffect->finit();

        DBG("done");
        for (int i=0;i<4;i++){
            mutexes[i]->try_lock();
        }
        delete buffer1;
        delete buffer2;
        delete vpout;
        if (play_worker)
            delete play_worker;
    }
    track_channels = 2;
    track_samples = 44100;

    buffer1 = new float[VPlayer::BUFFER_FRAMES*track_channels];
    buffer2 = new float[VPlayer::BUFFER_FRAMES*track_channels];

    play_worker = NULL;

    vpout = (VPOutPlugin *) new VPOutPluginAlsa();
    vpout->init(this, 44100, 2);
    vpeffect = (VPEffectPlugin *) new VPEffectPluginEQ();
    vpeffect->init(this);
    DBG("sadf");
}

int VPlayer::play()
{
    work = true;
    vpout->resume();
    if (play_worker == NULL){
        play_worker = new std::thread(VPlayer::play_work,this);
    }

    ((VPlayer *) this)->mutexes[0]->unlock();
    ((VPlayer *) this)->mutexes[2]->unlock();

}

void VPlayer::pause()
{
    vpout->pause();
    while (((VPlayer *) this)->mutexes[0]->try_lock() ||
           ((VPlayer *) this)->mutexes[1]->try_lock() ||
           ((VPlayer *) this)->mutexes[2]->try_lock() ||
           ((VPlayer *) this)->mutexes[3]->try_lock() ){}
}

void VPlayer::stop()
{
    setPosition(0);
    work = false;

    play_worker->join();

    prepare();
     DBG("ad");
}

