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
#include "vrok.h"
#include "config.h"
#include "vplayer.h"
#include "out.h"
#include "decoder.h"
#include "effect.h"

void VPlayer::play_work(VPlayer *self)
{
    while (1) {
        self->vpdecode->reader();
        ATOMIC_CAS(&self->active,true,false);
        self->next_track[0]='\0';


        if (self->next_track_cb && self->work) {
            self->next_track_cb(self->next_track, self->next_cb_user);

            if (self->next_track[0]!='\0') {
                self->open(self->next_track);
                self->active = true;
                DBG("new track");
                self->vpout->resume();
            } else {
                self->mutex_control.lock();
                self->play_worker = NULL;
                self->mutex_control.unlock();
                break;
            }
        } else {
            DBG("no new track");
            self->mutex_control.lock();
            self->play_worker = NULL;
            self->mutex_control.unlock();
            break;
        }
    }
    DBG("play worker dying");
    self->announce(VP_STATUS_STOPPED);

}

void VPlayer::addEffect(VPEffectPlugin *eff)
{
    eff->init(this);
    mutex_post_process.lock();
    effect_entry_t e;
    e.eff = eff;
    e.active = true;
    effects.push_back(e);
    mutex_post_process.unlock();
}
bool VPlayer::isActiveEffect(VPEffectPlugin *eff)
{
    if (eff) {
        for (std::vector<effect_entry_t>::iterator it=effects.begin();
             it!=effects.end();it++){
            if ((*it).eff==eff)
                return true;
        }
    }
    return false;
}
void VPlayer::removeEffect(VPEffectPlugin *eff)
{
    if (eff) {
        for (std::vector<effect_entry_t>::iterator it=effects.begin();
             it!=effects.end();it++){
            if ((*it).eff==eff){
                mutex_post_process.lock();
                effects.erase(it);
                eff->finit();
                mutex_post_process.unlock();
                break;
            }
        }
    }
}
void VPlayer::announce(VPStatus status)
{
    mutex_post_process.lock();
    for (std::vector<effect_entry_t>::iterator it=effects.begin();
         it!=effects.end();it++){
        (*it).eff->status_change(status);
    }
    mutex_post_process.unlock();
}

VPlayer::VPlayer(next_track_cb_t cb, void *cb_user)
{
    mutex_control.try_lock();
    next_cb_user = cb_user;

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

    play_worker_done = false;

    track_channels = 0;
    track_samplerate = 0;

    mutex_control.unlock();
    config_init();
}

int VPlayer::getSupportedFileTypeCount()
{
    return (int)sizeof(vpdecoder_entries)/sizeof(vpdecoder_entry_t);
}

void VPlayer::getSupportedFileTypeExtensions(char **exts)
{
    for (unsigned i=0;i<sizeof(vpdecoder_entries)/sizeof(vpdecoder_entry_t);i++){
        exts[i] = vpdecoder_entries[i].ext;
    }
}
int VPlayer::open(const char *url)
{

    if (vpdecode){
        vpout->resume();
        paused=false;
        DBG("free decoder");
        ATOMIC_CAS(&work,true,false);
        while (ATOMIC_CAS(&active,true,true)) {}
        delete vpdecode;
        vpdecode = NULL;
        vpout->rewind();
    }
    mutex_control.lock();
    this_track[0]='\0';


    unsigned len = strlen(url);
    for (unsigned i=0;i<sizeof(vpdecoder_entries)/sizeof(vpdecoder_entry_t);i++){
        if (strcasecmp(url + len - strlen(vpdecoder_entries[i].ext),vpdecoder_entries[i].ext) == 0) {
            DBG("open decoder "<<vpdecoder_entries[i].name);
            vpdecode = (VPDecoder *)vpdecoder_entries[i].creator();
            break;
        }
    }

    int ret = 0;

    if (vpdecode){
        vpdecode->init(this);
        ret += vpdecode->open(url);
    } else {
        ret = -1;
    }

    work = true;
    active = true;

    if (!play_worker){
        play_worker = new std::thread((void(*)(void*))VPlayer::play_work, this);
        play_worker_done = false;
        paused = false;
        DBG("make play worker");
        vpout->resume();
    }

    mutex_control.unlock();
    announce(VP_STATUS_OPEN);

    strcpy(this_track, url);
    return ret;
}
int VPlayer::play()
{
    mutex_control.lock();
    if (vpdecode && paused && active) {
        paused = false;
        vpout->resume();
        announce(VP_STATUS_PLAYING);
    }
    mutex_control.unlock();

    return 0;
}

void VPlayer::pause()
{
    mutex_control.lock();
    if (vpdecode && !paused && active) {
        vpout->pause();
        paused = true;
        announce(VP_STATUS_PAUSED);
    }
    mutex_control.unlock();

}
bool VPlayer::isPlaying()
{
    return !paused && active;
}

VPlayer::~VPlayer()
{
    if (vpdecode){
        vpout->resume();
        ATOMIC_CAS(&work,true,false);
        while (ATOMIC_CAS(&active,true,true)) {}
        delete vpdecode;
        vpdecode = NULL;
        vpout->rewind();
    }

    for (std::vector<effect_entry_t>::iterator it=effects.begin();
         it!=effects.end();
         it++){
        if (it->active){
            it->active = false;
            it->eff->finit();
            delete it->eff;
        }
    }

    if (vpout)
        delete vpout;
    if (buffer1)
        delete[] buffer1;
    if (buffer2)
        delete[] buffer2;
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

    if (!gapless_compatible) {
        if (vpout){
            delete vpout;
            vpout=NULL;
        }

        if (buffer1)
            delete[] buffer1;
        if (buffer2)
            delete[] buffer2;

        buffer1 = new float[VPBUFFER_FRAMES*track_channels];
        buffer2 = new float[VPBUFFER_FRAMES*track_channels];

        for (unsigned i=0;i<VPBUFFER_FRAMES*track_channels;i++){
            buffer1[i]=0.0f;
            buffer2[i]=0.0f;
        }

        DBG("Init sound output on "<< vpout_entries[DEFAULT_VPOUT_PLUGIN].name);
        vpout = (VPOutPlugin *) vpout_entries[DEFAULT_VPOUT_PLUGIN].creator();

        DBG("track chs:"<<track_channels);
        DBG("track rate:"<<track_samplerate);

        for (std::vector<effect_entry_t>::iterator it=effects.begin();
             it!=effects.end();
             it++){
            if (!it->active){
                it->eff->init(this);
                it->active=true;
            } else {
                it->eff->finit();
                it->eff->init(this);
            }
        }

        mutexes[1].lock();
        mutexes[3].lock();
        mutexes[0].try_lock();
        mutexes[0].unlock();
        mutexes[2].try_lock();
        mutexes[2].unlock();

        vpout->init(this, track_samplerate, track_channels);

    }

}

void VPlayer::setVolume(float vol)
{
    volume = vol;
}

float VPlayer::getVolume()
{
    return volume;
}


void VPlayer::post_process(float *buffer)
{
    mutex_post_process.lock();
    if (effects_active){
        for (std::vector<effect_entry_t>::iterator it=effects.begin();
             it!=effects.end();it++){
            if (it->active)
                it->eff->process( buffer);
        }
    }
    mutex_post_process.unlock();
}

