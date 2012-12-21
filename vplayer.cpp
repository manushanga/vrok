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

void VPlayer::addEffect(VPEffectPlugin *eff)
{
    effects.push_back(eff);
}
void VPlayer::removeEffect(unsigned idx)
{
    delete effects.at(idx);
    effects.erase(effects.begin()+idx);

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

    play_worker = NULL;
    work=false;
    effects_active=false;

    paused = true;
    volume = 1.0f;

    vpout=NULL;
    gapless_compatible = false;
    effects.clear();

    mutex_control->unlock();
}

int VPlayer::play()
{
    if (paused) {
        work = true;
        mutex_control->lock();
        paused = false;
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
        mutex_control->unlock();
    }
}
bool VPlayer::isPlaying()
{
    return !paused;
}
void VPlayer::ended()
{

}

void VPlayer::stop()
{

}

void VPlayer::post_process(float *buffer)
{
    if (effects_active){
        for (std::vector<VPEffectPlugin *>::iterator it=effects.begin();it!=effects.end();it++) {
            (*it)->process(buffer);
        }
    }

}

VPlayer::~VPlayer()
{

}
void VPlayer::set_metadata(unsigned samplerate, unsigned channels)
{
    if (track_channels == channels && track_samplerate == samplerate)
        gapless_compatible = true;
    else
        gapless_compatible = false;
    track_samplerate = samplerate;
    track_channels = channels;
}
void VPlayer::setVolume(float vol)
{
    volume = vol;
}

float VPlayer::getVolume()
{
    return volume;
}
int VPlayer::vpout_open()
{
    int ret;
    mutex_control->lock();

    mutexes[0]->unlock();
    mutexes[1]->try_lock();
    mutexes[2]->unlock();
    mutexes[3]->try_lock();

    if (buffer1)
        delete buffer1;
    if (buffer2)
        delete buffer2;

    buffer1 = new float[VPlayer::BUFFER_FRAMES*track_channels];
    buffer2 = new float[VPlayer::BUFFER_FRAMES*track_channels];


    if (vpout)
        delete vpout;
    vpout = (VPOutPlugin *) new VPOutPluginAlsa();
    DBG(track_channels);
    DBG(track_samplerate);
    DBG(vpout->getChannels());
    DBG(vpout->getSamplerate());

    if (!gapless_compatible){
        for (std::vector<VPEffectPlugin *>::iterator it=effects.begin();it!=effects.end();it++) {
            ret += (*it)->init(this);
        }

        if (vpout->getChannels() >= track_channels && vpout->getSamplerate() >= track_samplerate)
            ret += vpout->init(this, track_samplerate, track_channels);
        else if (vpout->getSamplerate() < track_samplerate){
            DBG("Can not initialize hardware for this samplerate");
        } else {
            DBG("Dropping channels due to hardware incompatibility");
            track_channels = vpout->getChannels();
            ret += vpout->init(this, track_samplerate, track_channels);
        }

    }

    mutex_control->unlock();
    return ret;
}
int VPlayer::vpout_close()
{
    mutex_control->lock();

    if(paused)
        play();

    work=false;

    vpout->pause();

    // let play_worker roll, make sure that only these mutexes are locked once,
    // if its done multiple times include the work check in between them. see
    // player_flac.cpp's worker function

    mutexes[0]->unlock();
    mutexes[2]->unlock();

    play_worker->join();
    DBG("player thread joined");
    if (vpout){
        vpout->finit();
        delete vpout;
        vpout=NULL;
    }
    mutex_control->unlock();

}
