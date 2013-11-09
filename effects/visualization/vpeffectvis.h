#ifndef VPEFFECTVIS_H
#define VPEFFECTVIS_H

#include "effect.h"

class VPEffectPluginVis : public VPEffectPlugin
{
private:
    enum vis_t{SCOPE, SPECTRUM};
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
    void toggleType() { if (type == SPECTRUM) type = SCOPE; else type=SPECTRUM; }
    void statusChange(VPStatus status) {status;}
    void minimized(bool state) { wstate = state; }
    int finit() ;
    ~VPEffectPluginVis() {}
};

#endif // VPEFFECTVIS_H
