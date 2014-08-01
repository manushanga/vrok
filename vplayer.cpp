/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
/*

  Notes:

  Decoders run only inside playWork function, each playback of a file runs in a
  new thread. This thread is destroyed soon after a file ends playing. But
  output runs continuously on the same thread IF gapless requirements hold
  (channel count and samplerate are the same in the next file). VPBuffer system
  consists of two buffers that are swapped after a fill. This happens in all
  buffer in the chain; for example,

  bout -> dsp_buf1 -> dsp_buf2 -> dsp_buf3 ... dsp_bufN -> bin

  All buffers swap when VP_SWAP_BUFFERS is used. The dual buffer system was
  introduced to lower the time that Vrok have to spend on the write lock. With
  this method we only spend very less time and actual data is write outside the
  lock.

*/
#include <cstring>
#include <algorithm>
#include "vrok.h"
#include "vplayer.h"
#include "out.h"
#include "decoder.h"
#include "effect.h"
#include "events.h"

void VPlayer::playWork(VPlayer *self)
{
    self->vpout->wakeup();
    while (1) {
        self->vpdecode->reader();

        ATOMIC_CAS(&self->active,true,false);
        DBG("waiting for output writing to finish");
        self->mutex[0].lock();
        self->mutex[0].unlock();
        DBG("done");
        self->nextResource.setURL("");


        if ( self->work) {

            VPEvents::getSingleton()->fire("GrabNext",&self->nextResource,sizeof(VPResource));

            if (self->nextResource.getURL().size() > 0) {
                self->open(self->nextResource,true);
                DBG("new track arrived");
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

}

VPlayer::VPlayer()
{
    control.lock();

    eff_count = 0;
    playWorker = NULL;
    work=false;
    active=false;

    paused = true;

    vpout=NULL;
    vpdecode=NULL;

    bout.chans = 0;
    bout.srate = 0;
    bout.buffer[0] = NULL;
    bout.buffer[1] = NULL;

    bufferCursor = 0;

    mutex[1].try_lock();

    VPEvents::getSingleton()->addEvent("GrabNext",1);
    VPEvents::getSingleton()->addEvent("StateChangePlaying",0);
    VPEvents::getSingleton()->addEvent("StateChangePaused",0);
    VPEvents::getSingleton()->addEvent("StateChangeStopped",0);
    VPEvents::getSingleton()->addEvent("StateChangeWindowMaximized",0);
    VPEvents::getSingleton()->addEvent("ErrorFileOpenFailure",0);

    control.unlock();

}


void VPlayer::setEffectsList(std::vector<VPEffectPlugin *> list)
{

    control.lock();
	bool wasPlaying = isPlaying();
	float pos = getPosition();
    stop();

    for (int i=0;i<eff_count;i++) {
        effects[i].eff->finit();
    }

    if (list.size() > VP_MAX_EFFECTS) {
        DBG("Too much effects!");
        eff_count=0;
        return;
    }

    eff_count=list.size();
    for (int i=0;i<eff_count;i++) {
        effects[i].eff = list[i];
        effects[i].active = true;
    }

    control.unlock();
    // we initialize DSP, this might be reinitialized if the srate and chans
    // are different in the new track, for this function this never happens
    if (currentResource.getURL().size() > 0) {
        initializeEffects();

        VPResource res=currentResource;
        open(res);
		
		pause();
		setPosition(pos);
		
		if (wasPlaying) {
			play();
		}
    }

}

std::vector<VPEffectPlugin *> VPlayer::getEffectsList()
{
    std::vector<VPEffectPlugin *> ll;
    for (int i=0;i<eff_count;i++) {
        ll.push_back(effects[i].eff);
    }
    return ll;
}

bool VPlayer::isEffectActive(VPEffectPlugin *eff)
{
    if (eff) {
        for (int i=0;i<eff_count;i++){
            if (effects[i].eff==eff && effects[i].active)
                return true;
        }
    }
    return false;
}

void VPlayer::initializeEffects()
{
    VPBuffer *tmp=&bout;
    for (int i=0;i<eff_count;i++){
        effects[i].in = tmp;
        effects[i].eff->init(this, tmp, &tmp);
        effects[i].out = tmp;
        if (!effects[i].active){
            effects[i].eff->finit();
        }
    }
    memcpy(&bin,tmp,sizeof(VPBuffer));
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
    int x=(int) state;
    VPEvents::getSingleton()->fire("StateChangeWindowMaximized",&x,sizeof(int));
}
int VPlayer::open(VPResource resource, bool tryGapless)
{

    control.lock();

    if (tryGapless) {
        delete vpdecode;
        vpdecode=NULL;
    } else {
        stop();
    }

    vpdecode=VPDecoderFactory::getSingleton()->create(resource,this);
    int ret = 0;

    if (vpdecode){
        ret = vpdecode->open(resource);
        if (ret < 0) {
            WARN("dropping file");
            delete vpdecode;
            vpdecode=NULL;
            control.unlock();
            return -1;
        }
    } else {
        WARN("failed to make vpdecoder");
        control.unlock();
        return -1;
    }

    work = true;
    active = true;

    FULL_MEMORY_BARRIER;

    if (!playWorker){
        playWorker = new std::thread((void(*)(void*))VPlayer::playWork, this);
        playWorker->high_priority();
        paused = false;
        DBG("make play worker");    
    }


    currentResource = resource;

    vpout->resume();

    control.unlock();

    // we started rolling so announce we are playing
    VPEvents::getSingleton()->fire("StateChangePlaying",NULL,0);

    return ret;
}
int VPlayer::play()
{
    control.lock();
    if (vpdecode && paused && active) {
        paused = false;
        vpout->resume();
    }
    control.unlock();
    VPEvents::getSingleton()->fire("StateChangePlaying",NULL,0);
    return 0;
}

void VPlayer::pause()
{
    control.lock();
    if (vpdecode && !paused && active) {
        vpout->pause();
        paused = true;
    }
    control.unlock();
    VPEvents::getSingleton()->fire("StateChangePaused",NULL,0);
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
    return active && !paused;
}

VPlayer::~VPlayer()
{
    control.lock();
    // stop the whole thing
    stop();
    control.unlock();
    // we do not free the effects plugins, their owners should free them
    for (int i=0;i<eff_count;i++){
        if (effects[i].active){
            effects[i].active = false;
            effects[i].eff->finit();
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
    bufferSamples[0] = 0;
    bufferSamples[1] = 0;
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

        bufferCursor =0;


        DBG("track chs: "<<bout.chans);
        DBG("track rate: "<<bout.srate);

        // bout - source to first dsp, output of input plugin
        // bin - source to vpout

        VPBuffer *tmp=&bout;
        DBG(eff_count);
        for (int i=0;i<eff_count;i++){
            effects[i].in = tmp;
            effects[i].eff->init(this, tmp, &tmp);
            effects[i].out = tmp;
            if (!effects[i].active){
                effects[i].eff->finit();
            }
        }
        memcpy(&bin,tmp,sizeof(VPBuffer));

        vpout->init(this, &bin);
    } else {
        DBG("gapless");
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
        ret= float(vpdecode->getPosition())/float(vpdecode->getLength());
    } else {
        ret= 0.0f;
    }
    //control.unlock();
    return ret;
}

void VPlayer::setNextTrackCallback(NextTrackCallback callback, void *user)
{
    nextTrackCallback = callback;
    nextCallbackUser = user;
}


void VPlayer::postProcess()
{

    for (int i=0;i<eff_count;i++){
        if (effects[i].active)
            effects[i].eff->process();
    }

}

