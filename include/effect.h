/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef EFFECT_H
#define EFFECT_H

#include "vplayer.h"
#include "vrok.h"
class VPEffectPlugin
{
protected:
    VPBuffer *bin,*bout;
public:
    virtual int init(VPlayer *v, VPBuffer *in, VPBuffer **out) = 0;
    virtual void process(float *buffer) = 0;
    virtual void statusChange(VPStatus status) { status; }
    virtual int finit() = 0;
    virtual void minimized(bool state) { state; } // for use for visual plugins
    virtual ~VPEffectPlugin() {}
};
#endif // EFFECT_H
