#include "vpeffectvis.h"
#include "ooura_fft.h"

VPEffectPluginVis::VPEffectPluginVis() : type(SCOPE), bars(NULL), wstate(false)
{
}

int VPEffectPluginVis::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{
    bars = new float[VPBUFFER_FRAMES];
    bin = in;
    *out = in;
    filled=false;
    return 0;
}

void VPEffectPluginVis::process(float *buffer)
{

    if (wstate)
        return;
    if (ATOMIC_CAS(&filled,true,true))
        return;

    for (int i=0;i<VPBUFFER_FRAMES;i++) {
        float mid=0.0f;
        for (int j=0;j<bin->chans;j++) {
            mid = (mid + buffer[i*bin->chans + j])/2.0f;
        }
        bars[i]=mid;
    }

    ATOMIC_CAS(&filled,false,true);
}

int VPEffectPluginVis::finit()
{
    if (bars)
        delete bars;
    return 0;
}

