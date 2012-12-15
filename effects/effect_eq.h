#ifndef EFFECT_EQ_H
#define EFFECT_EQ_H

#include "../vplayer.h"
#include "../effect.h"
#include "../shibatch/Equ.h"

class VPEffectPluginEQ : VPEffectPlugin {
private:
    VPlayer *owner;
    SuperEqState sb_state;
    float sb_preamp;
    void *sb_paramsroot;
    void sb_recalc_table();
public:
    int init(VPlayer *v);
    void process(float *buffer);
    int finit();
};

#endif
