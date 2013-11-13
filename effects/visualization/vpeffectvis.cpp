#include "vpeffectvis.h"
#include "ooura_fft.h"

VPEffectPluginVis::VPEffectPluginVis() : type(SCOPE), bars(NULL), wstate(false)
{
}

int VPEffectPluginVis::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{
    bin = in;
    bars = new float[VPBUFFER_FRAMES*bin->chans];
    ip= new int[int (sqrt(VPBUFFER_FRAMES*bin->chans/2.0f) ) +2];
    ip[0]=0;
    w = new float[VPBUFFER_FRAMES*bin->chans/2];
    for (int i=0;i<VPBUFFER_FRAMES*bin->chans;i++) { bars[i]=0.0f; }
    *out = in;
    return 0;
}

void VPEffectPluginVis::process(float *buffer)
{

    if (wstate)
        return;

    memcpy(bars,buffer,sizeof(float)*VPBUFFER_FRAMES*bin->chans);
    switch (type){
    case SCOPE:
        break;
    case SPECTRUM:
        rdft(VPBUFFER_FRAMES*bin->chans,1,bars,ip,w);
        break;
    default:
        break;
    }

}

int VPEffectPluginVis::finit()
{
    if (bars)
        delete bars;
    return 0;
}

