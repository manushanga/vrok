/*
  Vrok - smokin' audio
  (C) 20.0f12 Madura A. released under GPL 2.0.0f. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "config.h"
#include "eq.h"


VPEffectPluginEQ::VPEffectPluginEQ(float cap)
{
    sb_preamp = config_get_eq_preamp();
    sb_paramsroot = NULL;
    owner=NULL;
    config_get_eq_bands(target);
    memset(&sb_state, 0, sizeof(SuperEqState));

    bar_array = NULL;
    for (size_t i=0;i<BAR_COUNT;i++){
        trig[0][i]=(float *)new float[VPBUFFER_PERIOD*sizeof(float)];
        trig[1][i]=(float *)new float[VPBUFFER_PERIOD*sizeof(float)];
    }

    float fn, fi;
    for (size_t b=0;b<BAR_COUNT;b++){
        // lets just ASSUME that our buffer frame count is 44100
        fn = freqs[b]/22050.0f;
        freq_p[b]=fn;
        fi = PI*fn;
        for (size_t i=0;i<VPBUFFER_PERIOD;i++){
            trig[0][b][i]=cosf(fi*i);
            trig[1][b][i]=sinf(fi*i);
        }
    }

    for (unsigned i=0;i<BAR_COUNT+1;i++){
        mids[i]=0.0f;
        knowledge[i]=0.25f;
    }
    limit= cap;
    period_count = 0;
}

void VPEffectPluginEQ::status_change(VPStatus status){
    switch (status) {
        case VP_STATUS_OPEN:
        DBG("got status");
        for (unsigned i=0;i<BAR_COUNT;i++){
            knowledge[i]=0.25f;
        }
        sched_recalc=true;
        break;
        default:
        break;
    }
}
VPEffectPluginEQ::~VPEffectPluginEQ()
{
    for (size_t i=0;i<BAR_COUNT;i++){
        delete trig[0][i];
        delete trig[1][i];
    }
    if (bar_array)
        delete bar_array;
}
void VPEffectPluginEQ::sb_recalc_table()
{
    void *params = paramlist_alloc ();

    float bands_copy[BAR_COUNT];
    memcpy (bands_copy, sb_bands, sizeof (sb_bands));
    for (int i = 0; i < BAR_COUNT; i++) {
        bands_copy[i] *= sb_preamp;
    }

    equ_makeTable (&sb_state, bands_copy, params, (owner->track_samplerate==0?44100:owner->track_samplerate));
    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = params;

}

int VPEffectPluginEQ::init(VPlayer *v)
{
    if (bar_array)
        delete bar_array;
    bar_array = new float[BAR_COUNT*BAR_SETS];

    for (size_t i=0;i<BAR_COUNT*BAR_SETS;i++){
        bar_array[i] = 0.0f;
    }
    for (unsigned i=0;i<BAR_COUNT;i++){
        mids[i] = 0.0f;

        sb_bands[i] = 0.6f*target[i];
    }
    owner = v;
    equ_init (&sb_state, 10.0f, owner->track_channels);
    sb_recalc_table();
    return 0;

}
void VPEffectPluginEQ::applyKnowledge()
{
    for (unsigned i=0;i<BAR_COUNT;i++){
        sb_bands[i]=(knowledge[i]-mids[i]*0.0001f)*mids[i]*0.6f+sb_bands[i]*0.4f;
    }
    sched_recalc=true;
}
void VPEffectPluginEQ::process(float *buffer)
{
    if (sched_recalc){
        sb_recalc_table();
        sched_recalc=false;
    }

    equ_modifySamples_float(&sb_state, (char *)buffer, VPBUFFER_FRAMES, owner->track_channels);

    float mid,xre,xim,newb;
    unsigned step=0,chans=owner->track_channels,ichans;
    float *bar_array_w=bar_array;
    unsigned mid_write=0;
    while (step<BAR_SETS){
        for (size_t b=0;b<BAR_COUNT;b++){
            xre=0.0;
            xim=0.0;
            for (size_t i=0;i<VPBUFFER_PERIOD;i++){
                ichans = i*chans;
                mid = (buffer[ichans]+buffer[ichans+1] )* 0.5f;
                xre +=mid*trig[0][b][i];
                xim +=mid*trig[1][b][i];
            }
            newb = sqrtf((xre*xre + xim*xim)*(1.5f+ b*5.7f));
            bar_array_w[b] = (newb<limit)?newb:limit;
            mids[b]=1.0f+mids[b]*0.79f+bar_array_w[b]*0.2f;

        }
        for (unsigned b=0;b<BAR_COUNT;b++){
            for (unsigned h=0;h<10;h++){
                knowledge[b] -= 0.0002f*(mids[b]*knowledge[b]-target[b])*mids[b];
                if (knowledge[b]<0.01f)
                    knowledge[b]=0.025f;
            }
        }

        step+=1;
        bar_array_w += BAR_COUNT;
        buffer += VPBUFFER_PERIOD*owner->track_channels;
    }
    if (!period_count){
        applyKnowledge();
    }
    period_count = (period_count + BAR_SETS) % 30;

}

int VPEffectPluginEQ::finit()
{
    equ_quit(&sb_state);
    memset(&sb_state, 0, sizeof(SuperEqState));
    config_set_eq_bands(target);
    config_set_eq_preamp(sb_preamp);

    if (bar_array)
        delete bar_array;
    bar_array = NULL;

    return 0;
}
