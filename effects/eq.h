/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef EFFECT_EQ_H
#define EFFECT_EQ_H

#include "../effect.h"
#include "shibatch/Equ.h"

class VPEffectPluginEQ : VPEffectPlugin {
private:
    VPlayer *owner;
    SuperEqState sb_state;
    float sb_preamp;
    void *sb_paramsroot;
    void sb_recalc_table();
    bool work;
    float sb_bands[18];
public:
    VPEffectPluginEQ();
    const char **getBandNames();
    float *getBands();
    void setBand(int i, float val);
    void setPreamp(float val);
    float getPreamp();
    int init(VPlayer *v);
    void process(float *buffer);
    int finit();
    ~VPEffectPluginEQ();
};

#endif
