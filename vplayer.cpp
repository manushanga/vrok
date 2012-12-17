/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
/*
  Notes
  A file is played on play_worker thread, the mutexes[0], mutexes[2], mutex_pause
  control it's states. Pausing is done by invoking a try_lock() and the thread
  locks itself up next time it reaches mutex_pause->lock(), paused should be true.

  Only play_worker is managed here.
*/

#include <thread>

#include "vplayer.h"
#include "outs/out_alsa.h"
#include "effects/effect_eq.h"

void VPlayer::play_work(VPlayer *self)
{
    self->reader();
}
void VPlayer::reset()
{
    // gather play worker
    DBG("at reset");
    if (play_worker){
        work=false;

        // make sure its not locked
        paused = false;
        mutex_pause->unlock();
        mutexes[0]->unlock();
        mutexes[2]->unlock();

        play_worker->join();
        delete play_worker;
        play_worker = NULL;
    }
    // prepare muxtexes for new run
    mutexes[0]->try_lock();
    mutexes[1]->try_lock();
    mutexes[2]->try_lock();
    mutexes[3]->try_lock();

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
    mutex_control = new std::mutex();
    mutex_control->unlock();
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
        mutex_control->lock();

        paused = false;
        stopped = false;
        mutex_pause->unlock();
        mutexes[0]->unlock();
        mutexes[2]->unlock();

        if (play_worker == NULL){
            play_worker = new std::thread(VPlayer::play_work,this);
        }

        vpout->resume();
        mutex_control->unlock();

    }
    return 1;
}

void VPlayer::pause()
{
    if (paused ==false) {
        mutex_control->lock();
        vpout->pause();
        paused = true;
        mutex_pause->try_lock();
        while(mutexes[0]->try_lock()||
              mutexes[1]->try_lock()||
              mutexes[2]->try_lock()||
              mutexes[3]->try_lock()){}
        mutex_control->unlock();
    }
}
void VPlayer::ended()
{
    work=false;
    mutex_control->lock();

    mutex_control->unlock();
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

        mutex_control->lock();
        vpout->pause();
        stopped = true;
        mutex_pause->try_lock();
        while(mutexes[0]->try_lock()||
              mutexes[1]->try_lock()||
              mutexes[2]->try_lock()||
              mutexes[3]->try_lock()){}
        mutex_control->unlock();


    }
}

void VPlayer::post_process(float *buffer)
{
    if (effects)
        vpeffect->process(buffer);
}

VPlayer::~VPlayer()
{
    // we want to die a peacful death, free of locks!
    reset();
    DBG("VPLyer delete");

}


