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
#include "decoder.h"
#include "config.h"
#include "effect.h"

void VPlayer::play_work(VPlayer *self)
{
    self->vpdecode->reader();
}

void VPlayer::addEffect(VPEffectPlugin *eff)
{
    effect_entry e;
    e.eff = eff;
    e.init = false;
    effects.push_back(e);
}
void VPlayer::removeEffect(unsigned idx)
{
    std::list<effect_entry>::iterator it=effects.begin();
    for (int i=0;i<idx;i++){
        it++;
    }
    effects.erase(it,it);
}
void VPlayer::setGapplessCB(gapless_cb_t gcb)
{
    gapless_cb = gcb;
}
VPlayer::VPlayer()
{
    mutex_control.try_lock();

    track_channels = 0;
    track_samplerate = 0;

    buffer1 = NULL;
    buffer2 = NULL;

    mutexes[0].try_lock();
    mutexes[1].try_lock();
    mutexes[2].try_lock();
    mutexes[3].try_lock();

    play_worker = NULL;
    work=false;
    done=false;
    effects_active=false;

    paused = true;
    volume = 1.0f;

    vpout=NULL;
    vpdecode=NULL;

    gapless_compatible = false;
    next_track[0]='\0';
    gapless_cb = NULL;

    effects.clear();

    config_init();
    mutex_control.unlock();
}
int VPlayer::open(const char *url)
{
    mutex_control.lock();
    if (vpdecode){
        DBG("free decoder");
        delete vpdecode;
        vpdecode = NULL;
    }
    mutex_control.unlock();

    unsigned len = strlen(url);
    for (int i=0;i<sizeof(vpdecoder_entries)/sizeof(vpdecoder_entry_t);i++){
        if (strcmp(url + len - strlen(vpdecoder_entries[i].ext),vpdecoder_entries[i].ext) == 0) {
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
    paused = true;
    work=true;

    mutexes[0].try_lock();
    mutexes[1].unlock();
    mutexes[2].try_lock();
    mutexes[3].unlock();

    if (!play_worker){
        play_worker = new std::thread(VPlayer::play_work, this);
    }
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
    if (gapless_cb)
        gapless_cb(&next_track[0]);

    if (next_track[0]!='\0') {
        done = true;
        open(next_track);
        done = false;

        mutex_control.lock();
        vpout->resume();
        paused = false;
        mutex_control.unlock();

        VPlayer::play_work(this);
    }
}

void VPlayer::post_process(float *buffer)
{
    if (effects_active){
        for (std::list<effect_entry>::iterator it=effects.begin();it!=effects.end();it++) {
            (*it).eff->process(buffer);
        }
    }
}

VPlayer::~VPlayer()
{
    DBG("");
    if (vpdecode) {
        delete vpdecode;
        vpdecode = NULL;
    }
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
    DBG(done<<gapless_compatible);

    if (!(done && gapless_compatible)) {
        if (buffer1)
            delete buffer1;
        if (buffer2)
            delete buffer2;

        buffer1 = new float[VPlayer::BUFFER_FRAMES*track_channels];
        buffer2 = new float[VPlayer::BUFFER_FRAMES*track_channels];

        for (unsigned i=0;i<VPlayer::BUFFER_FRAMES*track_channels;i++){
            buffer1[i]=0.0f;
            buffer2[i]=0.0f;
        }

        if (vpout){
            delete vpout;
            vpout=NULL;
        }
        vpout = (VPOutPlugin *) config_get_VPOutPlugin_creator()();

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
        if (!(*it).init){
            ret += (*it).eff->init(this);
            (*it).init=true;
        }
    }

    return ret;
}
int VPlayer::vpout_close()
{
    vpout->pause();

    // let play_worker roll, make sure that only these mutexes are locked once,
    // if its done multiple times include the work check in between them. see
    // player_flac.cpp's worker function

    mutexes[0].unlock();
    mutexes[2].unlock();

    if (!done) {
        work = false;
        play_worker->join();
        DBG("player thread joined");
        delete play_worker;
        play_worker = NULL;
    }
}
