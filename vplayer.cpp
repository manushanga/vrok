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
#include <algorithm>
#include "vrok.h"
#include "vplayer.h"
#include "out.h"
#include "decoder.h"
#include "effect.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

void VPlayer::playWork(VPlayer *self)
{

#ifdef _WIN32
    assert(_CrtCheckMemory());
#endif
    self->vpout->wakeup();
    while (1) {
        self->vpdecode->reader();

        ATOMIC_CAS(&self->active,true,false);
        DBG("waiting for output writing to finish");
        self->mutex[0].lock();
        self->mutex[0].unlock();
        DBG("done");
        self->nextTrack[0]='\0';


        if (self->nextTrackCallback && self->work) {
            self->nextTrackCallback(self->nextTrack, self->nextCallbackUser);

            if (self->nextTrack[0]!='\0') {
                self->open(self->nextTrack);
                DBG("new track");
            } else {
                delete self->vpdecode;
                self->vpdecode = NULL;
                delete self->playWorker;
                self->playWorker = NULL;
                break;
            }
        } else {
            delete self->vpdecode;
            self->vpdecode = NULL;
            delete self->playWorker;
            self->playWorker = NULL;
            break;
        }
    }
    self->vpout->idle();
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

    mutex[1].try_lock();
    control.unlock();
}


void VPlayer::addEffect(VPEffectPlugin *eff)
{

    control.lock();
    stop();
    dsp[dspCount].in = NULL;
    dsp[dspCount].out = NULL;
    dsp[dspCount].eff = eff;
    dsp[dspCount].active = true;
    dspCount++;

    control.unlock();
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
    DBG(dspCount);
    stop();

    if (eff && dspCount > 0) {
        if (dspCount == 1 && eff == dsp[0].eff ) {
            memcpy(&bin,&bout,sizeof(VPBuffer));
            eff->finit();
            dspCount--;
        } else if (eff == dsp[dspCount-1].eff) {
            memcpy(&bin,&dsp[dspCount-2].out,sizeof(VPBuffer));
            eff->finit();
            dspCount--;
        } else {

            DBG("dsp removed");
            for (int i=0;i<dspCount-1;i++){
                if (dsp[i].eff==eff) {
                    eff->finit();

                    for (int j=i+1;j<dspCount-1;j++) {
                        memcpy(&dsp[j-1],&dsp[j],sizeof(VPBuffer));
                    }

                    int k=0;

                    VPBuffer *tmp;
                    tmp=&bout;
                    while (k<dspCount-1) {
                        dsp[k].eff->finit();
                        dsp[k].eff->init(this, tmp, &tmp);
                        k++;
                    }


                    memcpy(&bin,tmp,sizeof(VPBuffer));
                    delete vpout;

                    vpout =VPOutFactory::getSingleton()->create();
                    vpout->init(this,&bin);
                    dspCount--;

                    break;
                }
            }
        }
    }


    // we initialize DSP, this might be reinitialized if the srate and chans
    // are different in the new track, for this function this never happens
    if (wasPlaying && currentTrack[0]!='\0') {
        //initializeEffects();

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
    return VPDecoderFactory::getSingleton()->count();
}

void VPlayer::getSupportedFileTypeExtensions(std::vector<std::string>& exts)
{
    VPDecoderFactory::getSingleton()->getExtensionsList(exts);
}

void VPlayer::uiStateChanged(VPWindowState state)
{
    for (int i=0;i<dspCount;i++){
        dsp[i].eff->minimized(state == VPMINIMIZED);
    }
}
int VPlayer::open(const char *url)
{

    control.lock();

    stop();
    DBG(url);

    currentTrack[0]='\0';

    std::string ext;
    for (int i=strlen(url)-1;i>0;i--) {
        if (url[i]=='.'){
            break;
        } else {
            ext+=(url[i]);
        }
    }
    std::reverse(ext.begin(),ext.end());
    ext=std::to_lower(ext);
    vpdecode=VPDecoderFactory::getSingleton()->create(ext,this);
    int ret = 0;

    if (vpdecode){
        ret += vpdecode->open(url);
    } else {
        WARN("failed to make vpdecoder");
        ret = -1;
    }


    work = true;
    active = true;

    if (!playWorker){
        playWorker = new std::thread((void(*)(void*))VPlayer::playWork, this);
        playWorker->high_priority();
        paused = false;
        DBG("make play worker");    
    }

    announce(VP_STATUS_OPEN);

    strcpy(currentTrack, url);

    vpout->resume();

    control.unlock();

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
    // only run on control mutex locked
    if (ATOMIC_CAS(&active,true,true)) {
        if (vpdecode){

            DBG("free decoder");
            ATOMIC_CAS(&work,true,false);
            vpout->resume();

            playWorker->join();
            paused=true;

            delete playWorker;
            playWorker=NULL;
            delete vpdecode;
            vpdecode = NULL;
        }


        active = false;
    }
}
bool VPlayer::isPlaying()
{
    return !paused && ATOMIC_CAS(&active,true,true);
}

VPlayer::~VPlayer()
{
    control.lock();
    // stop the whole thing
    stop();
    control.unlock();
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
        ALIGNED_FREE(bout.buffer[0]);
        ALIGNED_FREE(bout.buffer[1]);
    }
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
            ALIGNED_FREE(bout.buffer[0]);
            bout.buffer[0] = NULL;
            ALIGNED_FREE(bout.buffer[1]);
            bout.buffer[1] = NULL;
        }

        vpout =VPOutFactory::getSingleton()->create();
        DBG(VPBUFFER_FRAMES);
        assert(vpout);

        outprop->buffer[0] = (float*)ALIGNED_ALLOC(sizeof(float)*VPBUFFER_FRAMES*outprop->chans);
        outprop->buffer[1] = (float*)ALIGNED_ALLOC(sizeof(float)*VPBUFFER_FRAMES*outprop->chans);
        outprop->cursor = &bufferCursor;

        assert(outprop->buffer[0] && outprop->buffer[1]);

        bout.chans = outprop->chans;
        bout.srate = outprop->srate;
        bout.buffer[0] = outprop->buffer[0];
        bout.buffer[1] = outprop->buffer[1];

        bout.cursor = &bufferCursor;

        DBG("track chs: "<<bout.chans);
        DBG("track rate: "<<bout.srate);

        // bout - source to first dsp, output of input plugin
        // bin - source to vpout

        VPBuffer *tmp=&bout;
        DBG(dspCount);
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
    volume = vol;
}

float VPlayer::getVolume()
{
    return volume;
}

void VPlayer::setPosition(float pos)
{
    control.lock();
    if (vpdecode) {
        //pause();
        vpdecode->setPosition((uint64_t)( pos*vpdecode->getLength() ) );
        //play();
    }
    control.unlock();
}

float VPlayer::getPosition()
{
    float ret;
    //control.lock();
    if (vpdecode) {
        ret= (vpdecode->getPosition()*1.0f)/(vpdecode->getLength()*1.0f);
    } else {
        ret= 0.0f;
    }
    //control.unlock();
    return ret;
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

