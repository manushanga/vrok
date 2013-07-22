/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef EFFECT_EQ_H
#define EFFECT_EQ_H

#include <cmath>

#include "effect.h"
#include "shibatch/Equ.h"

#include "vrok.h"
#include "config.h"

#define BACK_LOG 18
#define BAR_COUNT 18
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI M_PI
#define fFRAMES VPBUFFER_PERIOD*1.0f
#define BAR_SETS VPBUFFER_FRAMES/VPBUFFER_PERIOD

static const char *sb_bandnames[] = { "55 Hz", "77 Hz", "110 Hz",
    "156 Hz", "220 Hz", "311 Hz", "440 Hz", "622 Hz", "880 Hz", "1.2 kHz",
    "1.8 kHz", "2.5 kHz", "3.5 kHz", "5 kHz", "7 kHz", "10 kHz", "14 kHz",
    "20 kHz" };

static const float freqs[] = { 30.0f, 60.0f, 90.0f, 156.0f, 220.0f, 311.0f, 440.0f, 622.0f, 880.0f,
                               1200.0f, 1800.0f, 2500.0f, 3500.0f, 5000.0f, 7000.0f, 10000.0f, 14000.0f,
                               20000.0f };

class VPEffectPluginEQ : VPEffectPlugin {
private:
    VPlayer *owner;
    SuperEqState sb_state;
    unsigned log_write;
    float sb_preamp;
    void *sb_paramsroot;
    float sb_bands[BAR_COUNT] __attribute__ ((aligned(16)));
    float target[BAR_COUNT] __attribute__ ((aligned(16)));
    bool sched_recalc;
    float *bars[2];
    float *trig[2][BAR_COUNT] __attribute__ ((aligned(16)));
    float mids[BAR_COUNT] __attribute__ ((aligned(16)));
    float limit;
    float knowledge[BAR_COUNT] __attribute__ ((aligned(16)));
    float freq_p[BAR_COUNT];
    unsigned period_count;
    bool initd;
    void sb_recalc_table();
public:
    float *bar_array;
    VPEffectPluginEQ(float cap);
    void applyKnowledge();
    inline const char **getBandNames() const { return sb_bandnames; }
    inline const float *getBands() const { return sb_bands; }
    inline const float *getTargetBands() const {  return target; }
    inline const float *getMids() const { return mids; }
    inline void setTargetBand(int i, float val) { sb_bands[i]=val; target[i]=val; }
    inline void setBand(int i, float val) { sb_bands[i]=val; sched_recalc=true; }
    inline void setPreamp(float val) { sb_preamp = val; sched_recalc=true; }
    inline float getPreamp() const { return sb_preamp; }
    inline unsigned getBarCount() const { return BAR_COUNT; }
    inline unsigned getBarSetCount() const { return BAR_SETS; }

    int init(VPlayer *v, VPBuffer *in, VPBuffer **out);
    void statusChange(VPStatus status);
    void process(float *buffer);
    int finit();
    ~VPEffectPluginEQ();
};

#endif
