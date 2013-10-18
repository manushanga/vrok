/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#include "vrok.h"
#include "dsound.h"

LPDIRECTSOUND VPOutPluginDSound::lpds;
LPDIRECTSOUNDBUFFER VPOutPluginDSound::lpdsbuffer;
WAVEFORMATEX VPOutPluginDSound::wfx;
DSBUFFERDESC VPOutPluginDSound::dsbdesc;
HANDLE VPOutPluginDSound::NotifyEvent[2];
LPDIRECTSOUNDNOTIFY VPOutPluginDSound::lpDsNotify;
DSBPOSITIONNOTIFY VPOutPluginDSound::PositionNotify[2];

HRESULT VPOutPluginDSound::createSoundObject(void){
    HRESULT hr;

    hr = DirectSoundCreate(NULL,&lpds,NULL);
    hr = CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY);
    hr = lpds->SetCooperativeLevel(GetForegroundWindow(),DSSCL_PRIORITY);
    return hr;
}

WAVEFORMATEX VPOutPluginDSound::setWaveFormat(unsigned samplerate , unsigned channels){
    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = channels;
    wfx.wBitsPerSample = 16;
    wfx.nSamplesPerSec = samplerate;
    wfx.nBlockAlign     = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    return wfx;
}


DSBUFFERDESC VPOutPluginDSound::setBufferDescription(unsigned size){
    DSBUFFERDESC dsbdesc;
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS ;
    dsbdesc.dwBufferBytes = size;
    dsbdesc.lpwfxFormat = &wfx;
    return dsbdesc;
}


HRESULT VPOutPluginDSound::createSecondarySoundBuffer(void){
    HRESULT hr = lpds->CreateSoundBuffer(&dsbdesc,&lpdsbuffer,NULL);
    return hr;
}


VPOutPlugin* VPOutPluginDSound::VPOutPluginDSound_new()
{
    return (VPOutPlugin *) new VPOutPluginDSound();
}

void VPOutPluginDSound::worker_run(VPOutPluginDSound *self)
{
    LPVOID lpvWrite;
    DWORD  dwLength;

    HRESULT hr;

    hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

    if(SUCCEEDED(hr)){
        for (unsigned i=0;i<self->half_buffer_size*2;i++){
            ((short *)lpvWrite)[i]=0;
        }
        hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
        lpdsbuffer->SetCurrentPosition(0);
        lpdsbuffer->SetVolume(DSBVOLUME_MAX);
        lpdsbuffer->Play(0,0,DSBPLAY_LOOPING);
    } else {
        DBG("fail");
    }
    while (ATOMIC_CAS(&self->work,true,true)){
        if (ATOMIC_CAS(&self->pause_check,true,true)) {
            ATOMIC_CAS(&self->paused,false,true);
            lpdsbuffer->Stop();
            self->m_pause.lock();
            self->m_pause.unlock();
            lpdsbuffer->Play(0,0,DSBPLAY_LOOPING);
            ATOMIC_CAS(&self->paused,true,false);
            ATOMIC_CAS(&self->pause_check,true,false);
            if (!ATOMIC_CAS(&self->work,false,false)) {
                break;
            }

        }

        self->owner->mutex[1].lock();

        if (self->first_half){
            WaitForSingleObject(NotifyEvent[0], INFINITE);
            hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

            if(SUCCEEDED(hr)){
                for (unsigned i=0;i<self->half_buffer_size;i++){
                    ((short *)lpvWrite)[i]=(short)(self->bin->buffer[1-*self->bin->cursor][i]*32700.0f);
                }
                hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
            }
            self->first_half=0;
        } else {
            WaitForSingleObject(NotifyEvent[1], INFINITE);

            hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

            if(SUCCEEDED(hr)){
                for (unsigned i=0;i<self->half_buffer_size;i++){
                    ((short *)lpvWrite)[self->half_buffer_size+i]=(short)(self->bin->buffer[1-*self->bin->cursor][i]*32700.0f);
                }
                hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
            }

            self->first_half=1;
        }

        self->owner->mutex[0].unlock();

    }
    self->idle();
}

#if defined(_MSC_VER)
#pragma optimize("",off)
#elif defined(__gnuc__)
#pragma GCC push_options
#pragma GCC optimize ("O0")
#endif

void VPOutPluginDSound::rewind()
{

    if (!ATOMIC_CAS(&paused,false,false) ){
        m_pause.lock();     
        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
        first_half=1;
        idle();
    }
}

void VPOutPluginDSound::resume()
{
    if (ATOMIC_CAS(&paused,false,false) ){
        m_pause.unlock();
        while (ATOMIC_CAS(&paused,false,false)) {}
        wakeup();
    }
}
void VPOutPluginDSound::pause()
{
    if (!ATOMIC_CAS(&paused,false,false) ){
        m_pause.lock();
        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
        idle();
    }
}

#if defined(_MSC_VER)
#pragma optimize("",on)
#elif defined(__gnuc__)
#pragma GCC pop_options
#endif

void VPOutPluginDSound::wakeup()
{
    if (!wake){
        lpdsbuffer->Play(0,0,DSBPLAY_LOOPING);
        wake=true;
    }
}

void VPOutPluginDSound::idle()
{
    if (wake){
        lpdsbuffer->Stop();
        wake=false;
    }
}

int VPOutPluginDSound::init(VPlayer *v, VPBuffer *in)
{
    owner = v;
    bin = in;

    half_buffer_size = VPBUFFER_FRAMES*in->chans;
    first_half=1;

    HRESULT hr;
    createSoundObject();
    wfx = setWaveFormat(in->srate, in->chans);
    dsbdesc = setBufferDescription(VPBUFFER_FRAMES*in->chans*sizeof(short)*2);
    createSecondarySoundBuffer();

    NotifyEvent[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
    NotifyEvent[1] = CreateEvent(NULL,FALSE,FALSE,NULL);

    if (hr = lpdsbuffer->QueryInterface(IID_IDirectSoundNotify,(LPVOID*)&lpDsNotify) == DS_OK){
        PositionNotify[0].dwOffset = half_buffer_size*sizeof(short)-1;
        PositionNotify[0].hEventNotify = NotifyEvent[0];

        PositionNotify[1].dwOffset = half_buffer_size*sizeof(short)*2-1;
        PositionNotify[1].hEventNotify = NotifyEvent[1];

        if(hr = lpDsNotify->SetNotificationPositions((DWORD)2, PositionNotify) != DS_OK){
            DBG("Error while setting up Notification Positions!" );
            return -1;
        } else {
            lpDsNotify->Release();
        }        
    } else {
        return -1;
    }
    wake=true;
    work=true;
    paused=false;
    pause_check=false;
    worker = new std::thread( (void(*)(void*))worker_run, this);
    DBG("DSound thread made");
    return 0;
}

VPOutPluginDSound::~VPOutPluginDSound()
{
    // make sure decoders are finished before calling
    ATOMIC_CAS(&work,true,false);
    // make sure decoders have properly ended then mutex[0] should locked and
    // mutex[1] unlocked from the decoder and mutex[1] locked by output thread
    // we unlock it here to avoid deadlock

    // special lock release for DSound, the locking for output thread happens
    // twice at mutex[1] and WaitForSingleObject() we should release both to get
    // out of lock, the release is done in reverse to guarantee that both get
    // released
    lpdsbuffer->Play(0,0,DSBPLAY_LOOPING);
    owner->mutex[1].unlock();
    resume();

    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
    lpdsbuffer->Stop();

    lpdsbuffer->Release();

    lpds->Release();

}

