#ifndef VPEFFECTVIS_H
#define VPEFFECTVIS_H

#include "effect.h"

class VPEffectPluginVis : public VPEffectPlugin
{
public:
    enum vis_t{SCOPE, SPECTRUM};
private:
    float *bars;
    vis_t type;
    VPBuffer *bin;
public:


    bool wstate;
    bool filled;
    VPEffectPluginVis();
    inline float *getBars(){ return bars; }
    int init(VPlayer *v, VPBuffer *in, VPBuffer **out) ;
    void process(float *buffer) ;
    inline void setType(vis_t t) {  type=t;   }
    inline vis_t getType() { return type; }
    void statusChange(VPStatus status) {status;}
    void minimized(bool state) { wstate = state; }
    int finit() ;
    ~VPEffectPluginVis() {}
};

#endif // VPEFFECTVIS_H
