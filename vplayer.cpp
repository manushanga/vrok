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
    mutex_play = new std::mutex();
    mutex_play->unlock();
    mutex_pause = new std::mutex();
    mutex_pause->unlock();

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
        mutex_play->lock();

        paused = false;
        stopped = false;
        mutex_pause->unlock();
        mutexes[0]->unlock();
        mutexes[2]->unlock();

        if (play_worker == NULL){
            play_worker = new std::thread(VPlayer::play_work,this);
        }

        vpout->resume();
        mutex_play->unlock();

    }
    return 1;
}

void VPlayer::pause()
{
    if (paused ==false) {
        mutex_play->lock();
        vpout->pause();
        paused = true;
        mutex_pause->try_lock();
        mutex_play->unlock();
    }
}
void VPlayer::ended()
{
    work=false;
    mutex_play->lock();

    mutex_play->unlock();
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

        mutex_play->lock();
        vpout->pause();
        stopped = true;
        mutex_pause->try_lock();
        while(mutexes[0]->try_lock()||
              mutexes[1]->try_lock()||
              mutexes[2]->try_lock()||
              mutexes[3]->try_lock()){}
        mutex_play->unlock();


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


