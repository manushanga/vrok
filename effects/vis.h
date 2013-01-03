#ifndef EFFECT_VIS_H
#define EFFECT_VIS_H

#include "../effect.h"
#include "../config_out.h"

#define BAR_COUNT 16
#define BAR_SETS VPBUFFER_FRAMES/VPBUFFER_PERIOD

class VPEffectPluginVis : VPEffectPlugin {
private:
    VPlayer *owner;

    float *bars[2];
    float *trig[2][BAR_COUNT];
    float limit;
public:
    float *bar_array;
    static const unsigned BARS = BAR_COUNT;
    std::mutex mutex_vis;

    VPEffectPluginVis(float cap);
    int init(VPlayer *v);
    unsigned getBarCount();
    unsigned getBarSetCount();
    unsigned getMiliSecondsPerSet();
    void process(float *buffer);
    int finit();
    ~VPEffectPluginVis();
};

#endif // EFFECT_VIS_H
