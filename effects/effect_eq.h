/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef EFFECT_EQ_H
#define EFFECT_EQ_H

#include "../vplayer.h"
#include "../effect.h"
#include "shibatch/Equ.h"

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
