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
#include <cstring>

#include "thread_compat.h"

#include "vplayer.h"
#include "config_out.h"
#include "config.h"
#include "out.h"
#include "decoder.h"
#include "effect.h"


void VPlayer::play_work(VPlayer *self)
{
    self->vpdecode->reader();
}

void VPlayer::addEffect(VPEffectPlugin *eff)
{
    mutex_post_process.lock();
    effect_entry e;
    e.eff = eff;
    eff->init(this);
    e.active = true;
    effects.push_back(e);
    mutex_post_process.unlock();
}
bool VPlayer::isActiveEffect(VPEffectPlugin *eff)
{
    if (eff) {
        for (std::list<effect_entry>::iterator it=effects.begin();it!=effects.end();it++){
            if ((*it).eff==eff){
                return true;
            }
        }
    }
    return false;
}
void VPlayer::removeEffect(VPEffectPlugin *eff)
{
    if (eff) {
        for (std::list<effect_entry>::iterator it=effects.begin();it!=effects.end();it++){
            if ((*it).eff==eff){
                mutex_post_process.lock();
                (*it).active=false;
                effects.erase(it);
                eff->finit();
                mutex_post_process.unlock();
                break;
            }
        }
    }
}

VPlayer::VPlayer(next_track_cb_t cb)
{
    mutex_control.try_lock();

    mutex_post_process.unlock();
    track_channels = 0;
    track_samplerate = 0;

    buffer1 = NULL;
    buffer2 = NULL;

    play_worker = NULL;
    work=false;
    effects_active=false;

    paused = true;
    volume = 1.0f;

    vpout=NULL;
    vpdecode=NULL;

    gapless_compatible = false;
    next_track_cb = cb;
    next_track[0]='\0';
    effects.clear();
    play_worker_done = false;

    config_init();
    mutex_control.unlock();
}
int VPlayer::open(const char *url)
{
    if (vpdecode){
        if (!paused) {
            vpout->rewind();
            paused = true;
        }

        DBG("free decoder");
        delete vpdecode;
        vpdecode = NULL;
    }

    mutexes[0].unlock();
    mutexes[1].try_lock();
    mutexes[2].unlock();
    mutexes[3].try_lock();

    unsigned len = strlen(url);
    for (int i=0;i<sizeof(vpdecoder_entries)/sizeof(vpdecoder_entry_t);i++){
        if (strcasecmp(url + len - strlen(vpdecoder_entries[i].ext),vpdecoder_entries[i].ext) == 0) {
            DBG("open decoder "<<vpdecoder_entries[i].name);
            vpdecode = (VPDecoder *)vpdecoder_entries[i].creator();
        }
    }

    int ret = 0;

    if (vpdecode){
        vpdecode->init(this);
        mutex_control.lock();
        ret += vpdecode->open(url);
        mutex_control.unlock();
    } else {
        ret = -1;
    }

    work=true;

    if (!play_worker){
        play_worker = new std::thread((void(*)(void*))VPlayer::play_work, this);
    }

    play();
    return ret;
}
int VPlayer::play()
{
    mutex_control.lock();
    if (vpdecode && paused) {
        paused = false;
        vpout->resume();
    }
    mutex_control.unlock();
    return 0;
}

void VPlayer::pause()
{
    mutex_control.lock();
    if (vpdecode && !paused) {
        vpout->pause();
        paused = true;
    }
    mutex_control.unlock();
}
bool VPlayer::isPlaying()
{
    return !paused;
}
void VPlayer::ended()
{
    next_track[0]='\0';

    // better do this quick
    if (next_track_cb)
        next_track_cb(next_track);

    if (next_track[0]!='\0') {
        play_worker_done=true;
        open(next_track);
        play_worker_done=false;

        mutex_control.lock();
        vpout->resume();
        paused = false;
        mutex_control.unlock();

        VPlayer::play_work(this);
    } else {
        std::thread *p=play_worker;
        play_worker = NULL;
        p->detach();
        p->~thread();
    }
}

void VPlayer::post_process(float *buffer)
{
    mutex_post_process.lock();
    if (effects_active){
        for (std::list<effect_entry>::iterator it=effects.begin();it!=effects.end();it++) {
            if ((*it).active)
                (*it).eff->process( buffer);
        }
    }
    mutex_post_process.unlock();
}

VPlayer::~VPlayer()
{
    DBG("");
    if (vpdecode) {
        delete vpdecode;
        vpdecode = NULL;
    }
    if (buffer1)
        delete buffer1;
    if (buffer2)
        delete buffer2;
    config_finit();
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
    int ret=0;
    DBG(gapless_compatible);
    if (!gapless_compatible) {
        if (buffer1)
            delete buffer1;
        if (buffer2)
            delete buffer2;

        buffer1 = new float[VPBUFFER_FRAMES*track_channels];
        buffer2 = new float[VPBUFFER_FRAMES*track_channels];

        for (unsigned i=0;i<VPBUFFER_FRAMES*track_channels;i++){
            buffer1[i]=0.0f;
            buffer2[i]=0.0f;
        }

        if (vpout){
            vpout->resume();
            delete vpout;
            vpout=NULL;
        }
        DBG("Init sound output on "<< vpout_entries[DEFAULT_VPOUT_PLUGIN].name);
        vpout = (VPOutPlugin *) vpout_entries[DEFAULT_VPOUT_PLUGIN].creator();

        DBG("track chs:"<<track_channels);
        DBG("track rate:"<<track_samplerate);
        DBG("hw max chs:"<<vpout->get_channels());
        DBG("hw max rate:"<<vpout->get_samplerate());

        if (vpout->get_channels() >= track_channels && vpout->get_samplerate() >= track_samplerate) {
            ret += vpout->init(this, track_samplerate, track_channels);
        } else if (vpout->get_samplerate() < track_samplerate){
            DBG("Can not initialize hardware for this samplerate");
        } else {
            DBG("Dropping channels because hardware is not capable");
            track_channels = vpout->get_channels();
            ret += vpout->init(this, track_samplerate, track_channels);
        }
    }
    for (std::list<effect_entry>::iterator it=effects.begin();it!=effects.end();it++) {
        if (!(*it).active){
            ret += (*it).eff->init(this);
            (*it).active=true;
        }
    }

    return ret;
}
int VPlayer::vpout_close()
{
    // let play_worker roll, make sure that only these mutexes are locked once,
    // if its done multiple times include the work check in between them. see
    // player_flac.cpp's worker function

    if (!play_worker_done && play_worker) {
        work = false;
        mutexes[0].unlock();
        mutexes[2].unlock();
        play_worker->join();
        DBG("player thread joined");
        delete play_worker;
        play_worker = NULL;
    }
}
