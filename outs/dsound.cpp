/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#include <dsound.h>
#include "vrok.h"

LPDIRECTSOUND lpds;
LPDIRECTSOUNDBUFFER lpdsbuffer;
WAVEFORMATEX wfx;
DSBUFFERDESC dsbdesc;
HANDLE NotifyEvent[2];
LPDIRECTSOUNDNOTIFY lpDsNotify;
DSBPOSITIONNOTIFY PositionNotify[2];

HRESULT createSoundObject(void){
    HRESULT hr;

    hr = DirectSoundCreate(NULL,&lpds,NULL);
    hr = CoInitializeEx(NULL, 0);
    hr = lpds->SetCooperativeLevel(GetForegroundWindow(),DSSCL_NORMAL);
    return hr;
}

WAVEFORMATEX setWaveFormat(unsigned samplerate , unsigned channels){
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


DSBUFFERDESC setBufferDescription(unsigned size){
    DSBUFFERDESC dsbdesc;
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS ;
    dsbdesc.dwBufferBytes = size;
    dsbdesc.lpwfxFormat = &wfx;
    return dsbdesc;
}


HRESULT createSecondarySoundBuffer(void){
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
    DWORD play_at;
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
        if (ATOMIC_CAS(&self->pause_check,true,true)){
            DBG("going to pause");
            ATOMIC_CAS(&self->paused,false,true);
            lpdsbuffer->Stop();
            self->m_pause.lock();
            self->m_pause.unlock();
            lpdsbuffer->Play(0,0,DSBPLAY_LOOPING);
            ATOMIC_CAS(&self->paused,true,false);
            DBG("out of pause");
            ATOMIC_CAS(&self->pause_check,true,false);
        }

        self->owner->mutexes[1].lock();
        if (lpdsbuffer->GetCurrentPosition(&play_at,NULL) == DS_OK && play_at < self->half_buffer_size*sizeof(short)-1){
            WaitForSingleObject(NotifyEvent[0], INFINITE);
            hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

            if(SUCCEEDED(hr)){
                for (unsigned i=0;i<self->half_buffer_size;i++){
                    ((short *)lpvWrite)[i]=(short)(self->owner->buffer1[i]*32700.0f);
                }
                hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
            }
        } else {
            WaitForSingleObject(NotifyEvent[1], INFINITE);

            hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

            if(SUCCEEDED(hr)){
                for (unsigned i=0;i<self->half_buffer_size;i++){
                    ((short *)lpvWrite)[self->half_buffer_size+i]=(short)(self->owner->buffer1[i]*32700.0f);
                }
                hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
            }
        }

        self->owner->mutexes[0].unlock();


        self->owner->mutexes[3].lock();
        if (lpdsbuffer->GetCurrentPosition(&play_at,NULL) == DS_OK && play_at > self->half_buffer_size*sizeof(short)-1){
            WaitForSingleObject(NotifyEvent[1], INFINITE);

            hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

            if(SUCCEEDED(hr)){
                for (unsigned i=0;i<self->half_buffer_size;i++){
                    ((short *)lpvWrite)[self->half_buffer_size+i]=(short)(self->owner->buffer2[i]*32700.0f);
                }
                hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
            }
        } else {
            WaitForSingleObject(NotifyEvent[0], INFINITE);
            hr = lpdsbuffer->Lock(0,dsbdesc.dwBufferBytes,&lpvWrite,&dwLength,NULL,NULL,DSBLOCK_ENTIREBUFFER);

            if(SUCCEEDED(hr)){
                for (unsigned i=0;i<self->half_buffer_size;i++){
                    ((short *)lpvWrite)[i]=(short)(self->owner->buffer2[i]*32700.0f);
                }
                hr = lpdsbuffer->Unlock(lpvWrite,dwLength,NULL,NULL);
            }

        }
        self->owner->mutexes[2].unlock();
    }
    lpdsbuffer->Stop();
}

void __attribute__((optimize("O0"))) VPOutPluginPulse::rewind()
{

    m_pause.lock();
    ATOMIC_CAS(&pause_check,false,true);

    owner->mutexes[0].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer1[i]=0.0f;
    owner->mutexes[1].unlock();
    owner->mutexes[2].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer2[i]=0.0f;
    owner->mutexes[3].unlock();

    while (!ATOMIC_CAS(&paused,false,false)) {}

}

void __attribute__((optimize("O0"))) VPOutPluginDSound::resume()
{
    if (ATOMIC_CAS(&paused,true,true)){
        ATOMIC_CAS(&pause_check,true,false);

        m_pause.try_lock();
        m_pause.unlock();
        while (ATOMIC_CAS(&paused,true,true)) {}
    }
}
void __attribute__((optimize("O0"))) VPOutPluginDSound::pause()
{
    if (!ATOMIC_CAS(&paused,false,false)){

        m_pause.lock();
        ATOMIC_CAS(&pause_check,false,true);
        while (!ATOMIC_CAS(&paused,false,false)) {}
    }
}

int VPOutPluginDSound::init(VPlayer *v, unsigned samplerate, unsigned channels)
{
    owner = v;

    wbuffer=new short[VPBUFFER_FRAMES*channels*2];
    half_buffer_size = VPBUFFER_FRAMES*channels;

    HRESULT hr;
    createSoundObject();
    wfx = setWaveFormat(samplerate, channels);
    dsbdesc = setBufferDescription(half_buffer_size*sizeof(short)*2);
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
        }else {
            lpDsNotify->Release();
        }
    }

    ATOMIC_CAS(&work,false,true);
    ATOMIC_CAS(&paused,true,false);
    ATOMIC_CAS(&pause_check,true,false);
    worker = new std::thread( (void(*)(void*))worker_run, this);
    DBG("waveout thread made");
    return 0;
}

VPOutPluginDSound::~VPOutPluginDSound()
{
    // make sure decoders are finished before calling
    ATOMIC_CAS(&work,true,false);
    resume();

    owner->mutexes[0].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer1[i]=0.0f;
    owner->mutexes[1].unlock();

    owner->mutexes[2].lock();
    for (unsigned i=0;i<VPBUFFER_FRAMES*owner->track_channels;i++)
        owner->buffer2[i]=0.0f;
    owner->mutexes[3].unlock();
    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
    delete[] wbuffer;
    lpdsbuffer->Stop();

    lpdsbuffer->Release();

    lpds->Release();
}

