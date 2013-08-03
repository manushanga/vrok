/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
/*
  Notes:
  * Please do not propose to use std::string, I hate it for backend usage.
  * There is no resampling or channel mixing support on the VPlayer and it is
    not planned. If it can't be played on the hardware you have, it won't be
    played. Ofcause it is open for anyone else to implement!
*/

#ifndef VPLAYER_H
#define VPLAYER_H

#include <vector>

#include "threads.h"
#include "vputils.h"

#define VP_MAX_EFFECTS 32

enum VPStatus{VP_STATUS_OPEN,VP_STATUS_PLAYING,VP_STATUS_PAUSED,VP_STATUS_STOPPED};

class VPOutPlugin;
class VPEffectPlugin;
class VPDecoder;

typedef void(*next_track_cb_t)(char *mem, void *user);

struct VPBuffer {
    unsigned srate;
    unsigned chans;
    float *buffer;
};

struct VPEffect {
    VPEffectPlugin *eff;
    bool active;
    VPBuffer *in;
    VPBuffer *out;
};

class VPlayer
{
private:
    next_track_cb_t nextTrackCallback;
    VPEffect dsp[VP_MAX_EFFECTS];
    int dspCount;

    bool active;
    void *nextCallbackUser;
    void initializeEffects();
    void announce(VPStatus status);
public:
    char currentTrack[256];
    // take lock on mutex_control when writing to this
    char nextTrack[256];

    // mutex[0..1] for buffer, mutex[2..3] for buffer2
    std::mutex mutex[2];

    // open, play, pause, stop event control all are considered as cirtical
    // sections, none run interleaved.
    std::mutex control;

    // internal, play_worker runs only if work==true, if not it MUST return
    bool work;

    // internal, paused state
    bool paused;

    // external, effects are on if true
    bool effectsActive;

    std::thread *playWorker;
    VPOutPlugin *vpout;
    VPDecoder *vpdecode;
    // bout: buffer out from vpdecoder
    // bin: buffer in for vpout
    VPBuffer bout,bin;

    VPlayer(next_track_cb_t cb, void *cb_user);

    // internal interface
    static void playWork(VPlayer *self);
    void postProcess(float *buffer);
    void setOutBuffers(VPBuffer *outprop, VPBuffer **out);

    // external interface
    int open(const char *url);
    int play();
    void pause();
    void stop();
    void setVolume(float vol);
    float getVolume();
    void addEffect(VPEffectPlugin *eff);
    bool isEffectActive(VPEffectPlugin *eff);
    void removeEffect(VPEffectPlugin *eff);
    bool isPlaying();
    int getSupportedFileTypeCount();
    void getSupportedFileTypeExtensions(char **exts);
    ~VPlayer() ;
};

#endif // VPLAYER_H
