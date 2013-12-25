/*
  Vrok - smokin' audio
  (C) 2013 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include "spatial.h"
#define CONV(x,y) (x+y)
#define WRAPINC(x,max) (x+1) % max
#define CLIP(x) max(min(x,1.0f),-1.0f)


void VPEffectPluginSpatial::setParameters(float h, float L, float l)
{
    params[SPATIAL_PARAM_H]=h;
    params[SPATIAL_PARAM_L]=L;
    params[SPATIAL_PARAM_D]=l;
    float lld=(L-l)*0.5;
    float llp=(L+l)*0.5;
    length_diff = sqrt(h*h + llp*llp) - sqrt(h*h + lld*lld);
}

VPEffectPluginSpatial::VPEffectPluginSpatial() : delayed(NULL), enabled(false), remainder(NULL)
{

    setParameters(VSettings::getSingleton()->readFloat("spatial_H",0.6096f),
                  VSettings::getSingleton()->readFloat("spatial_L",0.6858f),
                  VSettings::getSingleton()->readFloat("spatial_D",0.1524f));
}

void VPEffectPluginSpatial::statusChange(VPStatus status){
    switch (status) {
        case VP_STATUS_OPEN:
        DBG("got status");

        break;
        default:
        break;
    }
}
VPEffectPluginSpatial::~VPEffectPluginSpatial()
{
    VSettings::getSingleton()->writeFloat("spatial_H",params[SPATIAL_PARAM_H]);
    VSettings::getSingleton()->writeFloat("spatial_D",params[SPATIAL_PARAM_D]);
    VSettings::getSingleton()->writeFloat("spatial_L",params[SPATIAL_PARAM_L]);

}

int VPEffectPluginSpatial::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{

    owner = v;
    bin = in;
    bout = in;
    *out = bout;
    remainder = (float *) ALIGNED_ALLOC(VPBUFFER_FRAMES*bin->chans*sizeof(float));
    delayed = (float *) ALIGNED_ALLOC(VPBUFFER_FRAMES*bin->chans*sizeof(float));


    if (in->chans == 2) {
        enabled = true;
    }

    for (int i=0;i<VPBUFFER_FRAMES*bin->chans;i++) {
        remainder[i] = 0.0f;
        delayed[i] = 0.0f;
    }
    remainder_write=0;
    delayed_write=0;

    return 0;
}

void VPEffectPluginSpatial::process(float *buffer)
{

    if (enabled) {
        // effective delay
        float delay=(length_diff/340.29f)*bin->srate;
        if (delay > VPBUFFER_FRAMES) {
            DBG("delay larger than buffer size!");
            return;
        }
        float delay_floor_fl=(float)floor(delay);
        int delay_floor=(int)floor(delay);
        int delay_ceil=(int)ceil(delay);

        delayed_write=0;

        for (register int i=0;i<delay_ceil*2;i++) {
            delayed[i] = remainder[i];

        }

        //DBG(delay_ceil );
        for (register int i=delay_ceil*2;i<VPBUFFER_FRAMES*2;i++) {
            delayed[i] = buffer[i -delay_ceil*2 ]+(delay - delay_floor_fl)*( buffer[i-delay_floor*2] - buffer[i-delay_ceil*2]  );

        }

        int rest_start=VPBUFFER_FRAMES*2 - delay_ceil*2 ;

        for (int i = 0; i < delay_ceil*2; i++) {
            remainder[i]=buffer[rest_start + i];
        }

        // add delayed effect
        for (register int i=0;i<VPBUFFER_FRAMES*2;i+=1) {
            buffer[i]=CONV(buffer[i], delayed[i]*-0.5f);
        }

    }
}

int VPEffectPluginSpatial::finit()
{
    if (remainder)
        ALIGNED_FREE(remainder);
    remainder=NULL;

    if (delayed)
        ALIGNED_FREE(delayed);
    delayed=NULL;
    return 0;
}
