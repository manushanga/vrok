/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#include <cmath>

#include "config_out.h"
#include "vis.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI M_PI
#define fFRAMES VPBUFFER_FRAMES*1.0

VPEffectPluginVis::VPEffectPluginVis(float *bars, float cap)
{
    bar_array = bars;
    mutex_vis.unlock();
    DBG("s");
    for (size_t i=0;i<BARS;i++){
        bar_array[i] = 0.0f;
        trig[0][i]=(float *)new float[VPBUFFER_FRAMES*sizeof(float)];
        trig[1][i]=(float *)new float[VPBUFFER_FRAMES*sizeof(float)];
    }

    float fn, fi;
    for (size_t b=0;b<BARS;b++){
        fn = ((float) (b+2))/((float)(BARS+5));
        fi=PI*fn*fn;
        for (size_t i=0;i<VPBUFFER_FRAMES;i++){
            trig[0][b][i]=cosf(fi*i);
            trig[1][b][i]=sinf(fi*i);
        }
    }
    limit= cap;

}

int VPEffectPluginVis::init(VPlayer *v)
{
    owner = v;
    return 0;
}

void VPEffectPluginVis::process(float *buffer)
{
    float mid,xre,xim,newb;
    mutex_vis.lock();
    for (size_t b=0;b<BARS;b++){
        xre=0.0;
        xim=0.0;
        for (size_t i=0;i<VPBUFFER_FRAMES;i++){
            mid = (buffer[i*owner->track_channels]+buffer[i*owner->track_channels+1] )* 0.5f;
            xre +=mid*trig[0][b][i];
            xim +=mid*trig[1][b][i];
        }
        newb = sqrtf(xre*xre + xim*xim)*sqrt((b+1)*2.0f);
        if (bar_array[b] < 0.1f && bar_array[b] > 0.0f)
            bar_array[b] = 0.0f;
        else if (bar_array[b] >= newb)
            bar_array[b] -= 0.6f+bar_array[b]*0.2;
        else
            bar_array[b] = (newb>limit)?limit:newb;
    }
    mutex_vis.unlock();

}
int VPEffectPluginVis::finit()
{
    return 0;
}
VPEffectPluginVis::~VPEffectPluginVis()
{
    for (size_t i=0;i<BARS;i++){
        delete trig[0][i];
        delete trig[1][i];
    }
}
