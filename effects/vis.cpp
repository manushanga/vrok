#include <cmath>

#include "vis.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI M_PI
#define fFRAMES VPlayer::BUFFER_FRAMES*1.0

VPEffectPluginVis::VPEffectPluginVis(float *bars)
{
    bar_array = bars;
    mutex_vis.unlock();

    for (size_t i=0;i<BARS;i++){
        bar_array[i] = 0.0f;
        trig[0][i]=(float *)new float[VPlayer::BUFFER_FRAMES*sizeof(float)];
        trig[1][i]=(float *)new float[VPlayer::BUFFER_FRAMES*sizeof(float)];
    }

    float fn, fi;
    for (size_t b=0;b<BARS;b++){
        fn = ((float) (b+2))/((float)(BARS+5));
        fi=PI*fn*fn;
        for (size_t i=0;i<VPlayer::BUFFER_FRAMES;i++){
            trig[0][b][i]=cosf(fi*i);
            trig[1][b][i]=sinf(fi*i);
        }
    }

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
        for (size_t i=0;i<VPlayer::BUFFER_FRAMES;i++){
            mid = (buffer[i*owner->track_channels]+buffer[i*owner->track_channels+1] )* 0.5f;
            xre +=mid*trig[0][b][i];
            xim +=mid*trig[1][b][i];
        }
        newb = sqrtf(xre*xre + xim*xim)*sqrt((b+1)*2.0f);
        if (bar_array[b] < 0.1f && bar_array[b] > 0.0f)
            bar_array[b] = 0.0f;
        else if (bar_array[b] >= newb)
            bar_array[b] -= 0.25f+bar_array[b]*0.01;
        else
            bar_array[b] = newb;
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
