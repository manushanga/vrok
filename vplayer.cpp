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
        mutexes[i]->try_lock();
    }

    play_worker = NULL;
    work=false;
    effects=false;

    track_channels = 2;
    track_samples = 44100;

    buffer1 = new float[VPlayer::BUFFER_FRAMES*track_channels];
    buffer2 = new float[VPlayer::BUFFER_FRAMES*track_channels];

    vpout = (VPOutPlugin *) new VPOutPluginAlsa();
    vpout->init(this, 44100, 2);
    vpeffect = (VPEffectPlugin *) new VPEffectPluginEQ();
    vpeffect->init(this);
    paused = false;
    stopped = true;
}

int VPlayer::play()
{
    if (paused || stopped) {
        work = true;
        vpout->resume();
        if (play_worker == NULL){
            play_worker = new std::thread(VPlayer::play_work,this);
        }

        mutexes[0]->unlock();
        mutexes[2]->unlock();
        paused = false;
        stopped = false;
    }
    return 1;
}

void VPlayer::pause()
{
    if (paused ==false) {
        vpout->pause();
        while (mutexes[0]->try_lock() ||
               mutexes[2]->try_lock()){}
        paused = true;
    }
}
void VPlayer::ended()
{
    work=false;
    while (mutexes[0]->try_lock() ||
           mutexes[2]->try_lock() ){}
    // its going to die anyway
    play_worker= NULL;

}
void VPlayer::stop()
{
    if (stopped == false){
       // setPosition(0);

        work = false;
        play_worker->join();
        delete play_worker;
        play_worker = NULL;

        while (mutexes[0]->try_lock() ||
               mutexes[2]->try_lock() ){}

        stopped = true;
    }
}

void VPlayer::post_process(float *buffer)
{
    if (effects)
        vpeffect->process(buffer);
}

VPlayer::~VPlayer()
{
    work = false;
    if (play_worker)
        play_worker->join();
    DBG("VPLyer delete");

}


