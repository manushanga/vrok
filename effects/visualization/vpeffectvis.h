#ifndef VPEFFECTVIS_H
#define VPEFFECTVIS_H

#include "effect.h"

class VPEffectPluginVis : public VPEffectPlugin
{
public:
    enum vis_t{SCOPE, SPECTRUM};
private:
    float *bars;
    int *ip;
    float *w;
    vis_t type;
    VPBuffer *bin;
public:


    bool wstate;

    VPEffectPluginVis();
    inline float *getBars(){ return bars; }
    int init(VPlayer *v, VPBuffer *in, VPBuffer **out) ;
    void process(float *buffer) ;
    void setType(vis_t t) { type=t; }
    void statusChange(VPStatus status) {status;}
    void minimized(bool state) { wstate = state; }
    int finit() ;
    ~VPEffectPluginVis() {}
};

#endif // VPEFFECTVIS_H
