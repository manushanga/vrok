/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
/*
  Notes
  A file is played on playWorker thread, the mutex[0], mutex[2], mutex_pause
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

void VPlayer::playWork(VPlayer *self)
{
    while (1) {
        self->vpdecode->reader();
        ATOMIC_CAS(&self->active,true,false);
        self->nextTrack[0]='\0';


        if (self->nextTrackCallback && self->work) {
            self->nextTrackCallback(self->nextTrack, self->nextCallbackUser);

            if (self->nextTrack[0]!='\0') {
                self->open(self->nextTrack);
                DBG("new track");
            } else {
                delete self->playWorker;
                self->playWorker = NULL;
                break;
            }
        } else {
            DBG("no new track");
            delete self->playWorker;
            self->playWorker = NULL;
            break;
        }
    }
    DBG("play worker dying");
    self->announce(VP_STATUS_STOPPED);
}

VPlayer::VPlayer(next_track_cb_t cb, void *cb_user)
{
    control.lock();
    nextCallbackUser = cb_user;

    dspCount = 0;
    playWorker = NULL;
    work=false;
    active=false;
    effectsActive=false;

    paused = true;

    vpout=NULL;
    vpdecode=NULL;

    nextTrackCallback = cb;
    nextTrack[0]='\0';
    currentTrack[0]='\0';

    bout.chans = 0;
    bout.srate = 0;
    bout.buffer[0] = NULL;
    bout.buffer[1] = NULL;

    bufferCursor = 0;
    control.unlock();

    mutex[1].lock();
    config_init();
}


void VPlayer::addEffect(VPEffectPlugin *eff)
{
    stop();

    dsp[dspCount].in = NULL;
    dsp[dspCount].out = NULL;
    dsp[dspCount].eff = eff;
    dsp[dspCount].active = true;
    dspCount++;

    // we initialize DSP, this might be reinitialized if the srate and chans
    // are different in the new track, for this function this never happens
    if (currentTrack[0]!='\0') {
        initializeEffects();

        char copy[256];
        strcpy(copy,currentTrack);
        open(copy);
    }
}
bool VPlayer::isEffectActive(VPEffectPlugin *eff)
{
    if (eff) {
        for (int i=0;i<dspCount;i++){
            if (dsp[i].eff==eff && dsp[i].active)
                return true;
        }
    }
    return false;
}
void VPlayer::removeEffect(VPEffectPlugin *eff)
{
    // STHAP!
    bool wasPlaying=active;
    DBG(wasPlaying);
    stop();

    if (eff) {
        if (eff == dsp[0].eff) {
            memcpy(&bin,&bout,sizeof(VPBuffer));
            eff->finit();
            dspCount--;
        } else if (eff == dsp[dspCount-1].eff) {
            memcpy(&bin,&dsp[dspCount-2].out,sizeof(VPBuffer));
            eff->finit();
            dspCount--;
        } else {
            for (int i=0;i<dspCount-1;i++){
                if (dsp[i].eff==eff) {
                    eff->finit();
                    for (int j=i+1;j<dspCount-1;j++) {
                        memcpy(&dsp[j-1],&dsp[j],sizeof(VPBuffer));
                    }
                    dspCount--;

                    break;
                }
            }
        }
    }

    // we initialize DSP, this might be reinitialized if the srate and chans
    // are different in the new track, for this function this never happens
    if (wasPlaying && currentTrack[0]!='\0') {
        initializeEffects();

        char copy[256];
        strcpy(copy,currentTrack);
        open(copy);
        announce(VP_STATUS_OPEN);
    }

}
void VPlayer::initializeEffects()
{
    VPBuffer *tmp=&bout;
    for (int i=0;i<dspCount;i++){
        dsp[i].in = tmp;
        dsp[i].eff->init(this, tmp, &tmp);
        dsp[i].out = tmp;
        if (!dsp[i].active){
            dsp[i].eff->finit();
        }
    }
    memcpy(&bin,tmp,sizeof(VPBuffer));
}

void VPlayer::announce(VPStatus status)
{
    // pass status to all DSP plugins
    for (int i=0;i<dspCount;i++){
        dsp[i].eff->statusChange(status);
    }

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
    stop();
    DBG(url);

    currentTrack[0]='\0';


    unsigned len = strlen(url);
    for (unsigned i=0;i<sizeof(vpdecoder_entries)/sizeof(vpdecoder_entry_t);i++){
        if (strcasecmp(url + len - strlen(vpdecoder_entries[i].ext),vpdecoder_entries[i].ext) == 0) {
            DBG("open decoder "<<vpdecoder_entries[i].name);
            vpdecode = (VPDecoder *)vpdecoder_entries[i].creator(this);
            break;
        }
    }

    int ret = 0;

    if (vpdecode){
        ret += vpdecode->open(url);
    } else {
        ret = -1;
    }


    work = true;
    active = true;

    if (!playWorker){
        playWorker = new std::thread((void(*)(void*))VPlayer::playWork, this);
        paused = false;
        DBG("make play worker");    
    }

    announce(VP_STATUS_OPEN);

    strcpy(currentTrack, url);

    vpout->resume();
    return ret;
}
int VPlayer::play()
{
    control.lock();
    if (vpdecode && paused && active) {
        paused = false;
        vpout->resume();
        announce(VP_STATUS_PLAYING);
    }
    control.unlock();

    return 0;
}

void VPlayer::pause()
{
    control.lock();
    if (vpdecode && !paused && active) {
        vpout->pause();
        paused = true;
        announce(VP_STATUS_PAUSED);
    }
    control.unlock();

}

void VPlayer::stop()
{
    control.lock();
    if (active) {
        if (vpdecode){
            vpout->resume();
            paused=false;
            DBG("free decoder");
            ATOMIC_CAS(&work,true,false);
            while (ATOMIC_CAS(&active,true,true)) {}
            delete vpdecode;
            vpdecode = NULL;
        }

        if (vpout)
            vpout->rewind();

        // reset mutexes, both out and decoding is stopped here
        mutex[0].try_lock();
        mutex[0].unlock();
        mutex[1].try_lock();
        active = false;
    }

    control.unlock();
}
bool VPlayer::isPlaying()
{
    return !paused && ATOMIC_CAS(&active,true,true);
}

VPlayer::~VPlayer()
{

    // stop the whole thing
    stop();
    // we do not free the effects plugins, their owners should free them
    for (int i=0;i<dspCount;i++){
        if (dsp[i].active){
            dsp[i].active = false;
            dsp[i].eff->finit();
        }
    }

    if (vpout) {
        delete vpout;
        vpout=NULL;
    }
    if (bout.buffer[0]) {
        delete[] bout.buffer[0];
        delete[] bout.buffer[1];
    }
    config_finit();
}

void VPlayer::setOutBuffers(VPBuffer *outprop, VPBuffer **out)
{

    *out = &bout;
    bufferCursor = 0;
    if (bout.srate != outprop->srate || bout.chans != outprop->chans) {
        if (vpout){
            delete vpout;
            vpout=NULL;
        }

        if (bout.buffer[0]) {
            delete[] bout.buffer[0];
            bout.buffer[0] = NULL;
            delete[] bout.buffer[1];
            bout.buffer[1] = NULL;
        }

        outprop->buffer[0] = new float[VPBUFFER_FRAMES*outprop->chans];
        outprop->buffer[1] = new float[VPBUFFER_FRAMES*outprop->chans];
        outprop->cursor = &bufferCursor;

        bout.chans = outprop->chans;
        bout.srate = outprop->srate;
        bout.buffer[0] = outprop->buffer[0];
        bout.buffer[1] = outprop->buffer[1];

        bout.cursor = &bufferCursor;

        DBG("Init sound output on "<< vpout_entries[DEFAULT_VPOUT_PLUGIN].name);
        vpout = (VPOutPlugin *) vpout_entries[DEFAULT_VPOUT_PLUGIN].creator();

        DBG("track chs: "<<bout.chans);
        DBG("track rate: "<<bout.srate);

        VPBuffer *tmp=&bout;
        for (int i=0;i<dspCount;i++){
            dsp[i].in = tmp;
            dsp[i].eff->init(this, tmp, &tmp);
            dsp[i].out = tmp;
            if (!dsp[i].active){
                dsp[i].eff->finit();
            }
        }
        memcpy(&bin,tmp,sizeof(VPBuffer));

        vpout->init(this, &bin);
    }
}

void VPlayer::setVolume(float vol)
{
    //volume = vol;
}

float VPlayer::getVolume()
{
    //return volume;
}


void VPlayer::postProcess(float *buffer)
{
    if (effectsActive){
        for (int i=0;i<dspCount;i++){
            if (dsp[i].active)
                dsp[i].eff->process(buffer);
        }
    }
}

