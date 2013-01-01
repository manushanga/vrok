#ifndef EFFECT_VIS_H
#define EFFECT_VIS_H

#include "../effect.h"
#define BAR_COUNT 16

class VPEffectPluginVis : VPEffectPlugin {
private:
    VPlayer *owner;
    float *bar_array;
    float *bars[2];
    float *trig[2][BAR_COUNT];
    float limit;
public:
    static const unsigned BARS = BAR_COUNT;
    std::mutex mutex_vis;

    VPEffectPluginVis(float *bars, float cap);
    int init(VPlayer *v);
    void process(float *buffer);
    int finit();
    ~VPEffectPluginVis();
};

#endif // EFFECT_VIS_H
