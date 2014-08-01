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
	VPlayer *owner;
public:
    virtual int init(VPlayer *v, VPBuffer *in, VPBuffer **out) = 0;
    virtual void process() = 0;
    virtual int finit() = 0;
    virtual ~VPEffectPlugin() {}
};
#endif // EFFECT_H
