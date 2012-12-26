/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
/*
  Many thanks to the original author of this code that can be found at,
  http://read.pudn.com/downloads91/sourcecode/windows/multimedia/350420/rawaudio/rawaudio.c__.htm

  All this is messy, I know and I don't care.
*/

#include "waveout.h"   
/*
 * some good values for block size and count
 */
#define BLOCK_SIZE  8192
#define BLOCK_COUNT 20

/*
 * function prototypes
 */
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
static WAVEHDR* allocateBlocks(int size, int count);
static void freeBlocks(WAVEHDR* blockArray);
static void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
static int start(unsigned samplerate, unsigned channels);
static int stop();
/*
 * module level variables
 */
static CRITICAL_SECTION waveCriticalSection;
static WAVEHDR*         waveBlocks;
static volatile int     waveFreeBlockCount;
static int              waveCurrentBlock;

HWAVEOUT hWaveOut; /* device handle */
WAVEFORMATEX wfx;  /* look this up in your documentation */

VPOutPlugin* _VPOutPluginWaveOut_new()
{
    return (VPOutPlugin *) new VPOutPluginWaveOut();
}

static void worker_run(VPOutPluginWaveOut *self)
{

    DBG("");
    while (self->work){
        if (self->paused){
            // we do alsa syncs here, instead of the calling thread
            // because of thread syncing problems
            DBG("going to pause");
            self->mutex_pause->lock();
            DBG("out of pause");
        }

        self->owner->mutexes[1].lock();
        for (unsigned i=0;i<VPlayer::BUFFER_FRAMES*self->owner->track_channels;i++){
            self->wbuffer1[i]=(short)(self->owner->buffer1[i]*32768.0f);
        }
        writeAudio(hWaveOut,(char *) self->wbuffer1, VPlayer::BUFFER_FRAMES*self->owner->track_channels*sizeof(short));
        self->owner->mutexes[0].unlock();

        self->owner->mutexes[3].lock();
        for (unsigned i=0;i<VPlayer::BUFFER_FRAMES*self->owner->track_channels;i++){
            self->wbuffer2[i]=(short)(self->owner->buffer2[i]*32768.0f);
        }

        writeAudio(hWaveOut,(char *) self->wbuffer2, VPlayer::BUFFER_FRAMES*self->owner->track_channels*sizeof(short));
        self->owner->mutexes[2].unlock();
    }
}


void VPOutPluginWaveOut::resume()
{
    paused = false;
    mutex_pause->unlock();
}
void VPOutPluginWaveOut::pause()
{
    paused = true;
    mutex_pause->try_lock();
}

int VPOutPluginWaveOut::init(VPlayer *v, unsigned samplerate, unsigned channels)
{
    owner = v;

    wbuffer1=new short[VPlayer::BUFFER_FRAMES*channels];
    wbuffer2=new short[VPlayer::BUFFER_FRAMES*channels];
    start(samplerate, channels);
    mutex_pause = new std::mutex();
    mutex_pause->try_lock();
    paused=true;
    work=true;
    worker = new std::thread(worker_run, this);
    DBG("waveout thread made");
    return 0;
}

unsigned VPOutPluginWaveOut::get_samplerate()
{
    // problem?
    return 192001;
}
unsigned VPOutPluginWaveOut::get_channels()
{
    // another problem?
    return 8;
}
VPOutPluginWaveOut::~VPOutPluginWaveOut()
{
    work=false;
    if(!paused)
        pause();
    owner->mutexes[1].unlock();
    owner->mutexes[3].unlock();

    resume();
    if (worker){
        worker->join();
        DBG("out thread joined");
        delete worker;
    }
    delete mutex_pause;
    stop();
}
int stop()
{
    int i;
    /*
     * playback loop
     */
   /* while(1) {


        writeAudio(hWaveOut, buffer, sizeof(buffer));
    }*/

    /*
     * wait for all blocks to complete
     */
    while(waveFreeBlockCount < BLOCK_COUNT)
        Sleep(10);

    /*
     * unprepare any blocks that are still prepared
     */
    for(i = 0; i < waveFreeBlockCount; i++)
        if(waveBlocks[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, &waveBlocks[i], sizeof(WAVEHDR));


    DeleteCriticalSection(&waveCriticalSection);
    freeBlocks(waveBlocks);
    waveOutClose(hWaveOut);
    return 0;
}
int start(unsigned samplerate, unsigned channels)
{
    int i;

    /*
     * initialise the module variables
     */
    waveBlocks         = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
    waveFreeBlockCount = BLOCK_COUNT;
    waveCurrentBlock   = 0;

    InitializeCriticalSection(&waveCriticalSection);

    /*
     * set up the WAVEFORMATEX structure.
     */
    wfx.nSamplesPerSec  = 44100;  /* sample rate */
    wfx.wBitsPerSample  = 16;     /* sample size */
    wfx.nChannels       = 2;      /* channels    */
    wfx.cbSize          = 0;      /* size of _extra_ info */
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nBlockAlign     = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    /*
     * try to open the default wave device. WAVE_MAPPER is
     * a constant defined in mmsystem.h, it always points to the
     * default wave device on the system (some people have 2 or
     * more sound cards).
     */
    if(waveOutOpen(
        &hWaveOut,
        WAVE_MAPPER,
        &wfx,
        (DWORD_PTR)waveOutProc,
        (DWORD_PTR)&waveFreeBlockCount,
        CALLBACK_FUNCTION
    ) != MMSYSERR_NOERROR) {
        DBG("Opening WaveOut fail");
        return -1;
    }
    return 0;
}

static void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
    WAVEHDR* current;
    int remain;

    current = &waveBlocks[waveCurrentBlock];

    while(size > 0) {
        /*
         * first make sure the header we're going to use is unprepared
         */
        if(current->dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

        if(size < (int)(BLOCK_SIZE - current->dwUser)) {
            memcpy(current->lpData + current->dwUser, data, size);
            current->dwUser += size;
            break;
        }

        remain = BLOCK_SIZE - current->dwUser;
        memcpy(current->lpData + current->dwUser, data, remain);
        size -= remain;
        data += remain;
        current->dwBufferLength = BLOCK_SIZE;

        waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

        EnterCriticalSection(&waveCriticalSection);
        waveFreeBlockCount--;
        LeaveCriticalSection(&waveCriticalSection);

        /*
         * wait for a block to become free
         */
        while(!waveFreeBlockCount)
            Sleep(10);

        /*
         * point to the next block
         */
        waveCurrentBlock++;
        waveCurrentBlock %= BLOCK_COUNT;

        current = &waveBlocks[waveCurrentBlock];
        current->dwUser = 0;
    }
}

static WAVEHDR* allocateBlocks(int size, int count)
{
    unsigned char* buffer;
    int i;
    WAVEHDR* blocks;
    DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

    /*
     * allocate memory for the entire set in one go
     */
    if((buffer = (unsigned char *) HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        totalBufferSize
    )) == NULL) {
        //fprintf(stderr, "Memory allocation error\n");
        ExitProcess(1);
    }

    /*
     * and set up the pointers to each bit
     */
    blocks = (WAVEHDR*)buffer;
    buffer += sizeof(WAVEHDR) * count;
    for(i = 0; i < count; i++) {
        blocks[i].dwBufferLength = size;
        blocks[i].lpData = (char *) buffer;
        buffer += size;
    }

    return blocks;
}

static void freeBlocks(WAVEHDR* blockArray)
{
    /*
     * and this is why allocateBlocks works the way it does
     */
    HeapFree(GetProcessHeap(), 0, blockArray);
}

static void CALLBACK waveOutProc(
    HWAVEOUT hWaveOut,
    UINT uMsg,
    DWORD dwInstance,
    DWORD dwParam1,
    DWORD dwParam2
)
{
    int* freeBlockCounter = (int*)dwInstance;
    /*
     * ignore calls that occur due to openining and closing the
     * device.
     */
    if(uMsg != WOM_DONE)
        return;

    EnterCriticalSection(&waveCriticalSection);
    (*freeBlockCounter)++;
    LeaveCriticalSection(&waveCriticalSection);
}
