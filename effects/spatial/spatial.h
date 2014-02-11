/*
  Vrok - smokin' audio
  (C) 2013 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef SPATIAL_H
#define SPATIAL_H

#include <cmath>
#include "effect.h"

#define SPATIAL_PARAM_H 0
#define SPATIAL_PARAM_L 1
#define SPATIAL_PARAM_D 2

class VPEffectPluginSpatial : public VPEffectPlugin
{
private:
    float *remainder;
    float *delayed;
    int remainder_write;
    int delayed_write;
    bool enabled;
    float length_diff;
    float params[3];
public:
    float *getParameters() { return &params[0]; }
    void setParameters(float h, float L, float l);
    VPEffectPluginSpatial();
    void statusChange(VPStatus status);
    int init(VPlayer *v, VPBuffer *in, VPBuffer **out);
    void process();
    int finit();
    ~VPEffectPluginSpatial();
};

#endif // SPATIAL_H
