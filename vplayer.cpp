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
    mutex_pause->unlock();

    mutexes[0]->unlock();
    mutexes[1]->try_lock();
    mutexes[2]->unlock();
    mutexes[3]->try_lock();
}

VPlayer::VPlayer()
{
    mutex_control = new std::mutex();
    mutex_control->unlock();

    mutex_control->lock();
    track_channels = 0;
    track_samplerate = 0;

    buffer1 = NULL;
    buffer2 = NULL;

    for (int i=0;i<4;i++){
        mutexes[i] = new std::mutex();
        mutexes[i]->try_lock();
    }

    mutex_pause = new std::mutex();
    mutex_pause->try_lock();

    play_worker = NULL;
    work=false;
    effects=false;

    track_channels = 2;
    track_samplerate = 44100;

    buffer1 = new float[VPlayer::BUFFER_FRAMES*track_channels];
    buffer2 = new float[VPlayer::BUFFER_FRAMES*track_channels];

    vpout = (VPOutPlugin *) new VPOutPluginAlsa();
    vpout->init(this, 44100, 2);
    vpeffect = (VPEffectPlugin *) new VPEffectPluginEQ();
    vpeffect->init(this);
    paused = true;
    volume = 1.0f;
    mutex_control->unlock();
}

int VPlayer::play()
{
    if (paused) {
        work = true;
        mutex_control->lock();
        paused = false;
        mutex_pause->unlock();
        vpout->resume();
        if (!play_worker){
            play_worker = new std::thread(VPlayer::play_work, this);
        }
        mutex_control->unlock();
    }
    return 1;
}

void VPlayer::pause()
{
    if (!paused) {
        mutex_control->lock();
        vpout->pause();
        paused = true;
        mutex_pause->try_lock();
        mutex_control->unlock();
    }
}

void VPlayer::ended()
{

}

void VPlayer::stop()
{
    if (!paused) {
        mutex_control->lock();
        vpout->pause();
        paused = true;
        mutex_pause->try_lock();
        setPosition(0);
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

}

void VPlayer::setVolume(float vol)
{
    volume = vol;
}

float VPlayer::getVolume()
{
    return volume;
}
void VPlayer::vpout_open()
{

}
void VPlayer::vpout_close()
{
    if (vpout){
        vpout->finit();
        delete vpout;
        vpout=NULL;
    }
}
