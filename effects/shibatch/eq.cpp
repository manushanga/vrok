/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "vputils.h"
#include "eq.h"
#if defined(min) && defined(max)
#define CLIP(x) max(min(x,1.0f),-1.0f)
#else
#define CLIP(x) std::max(std::min(x,1.0f),-1.0f)
#endif
VPEffectPluginEQ::VPEffectPluginEQ(float cap)
{
    sb_preamp = (float) VSettings::getSingleton()->readFloat("eqpre",96.0f);
    autopreamp = (bool) VSettings::getSingleton()->readInt("autopreamp",0);
    sb_paramsroot = NULL;
    sched_recalc = false;
    owner=NULL;
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        target[i]=VSettings::getSingleton()->readFloat(band,pow(10,48.0f/-20));
    }
    memset(&sb_state, 0, sizeof(SuperEqState));
    limit= cap;
    initd=false;

}

void VPEffectPluginEQ::statusChange(VPStatus status){
    switch (status) {
        case VP_STATUS_OPEN:
        DBG("got status");

        break;
        default:
        break;
    }
}
VPEffectPluginEQ::~VPEffectPluginEQ()
{
    if (initd)
        finit();
}
void VPEffectPluginEQ::sb_recalc_table()
{
    void *params = paramlist_alloc ();


    equ_makeTable (&sb_state, sb_bands, params, bin->srate);
    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = params;

}

int VPEffectPluginEQ::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{

    for (unsigned i=0;i<BAR_COUNT;i++){
        sb_bands[i] = target[i];
    }

    owner = v;
    bin = in;
    bout = in;
    *out = in;

    equ_init (&sb_state, 14, bin->chans);
    sb_recalc_table();
    initd = true;

    return 0;

}
void VPEffectPluginEQ::applyKnowledge()
{

/*    for (unsigned i=0;i<BAR_COUNT;i++){
        float next=knowledge[i]*mids[i]*0.6f + sb_bands[i]*0.4f;
        if (next<target[i] && next>target[i]-3.0f){
            sb_bands[i] = next;
        } else {
            knowledge[i]*=0.5f;
        }
    }
    sched_recalc=true;
*/
}
void VPEffectPluginEQ::process(float *buffer)
{
    if (sched_recalc){
        sb_recalc_table();
        sched_recalc=false;
    }

    //assert(buffer == bin->buffer);

    equ_modifySamples_float(&sb_state, (char *)buffer, VPBUFFER_FRAMES, bin->chans);


    if (autopreamp) {
        bool ff=true, xx=true;
        for (register int i=0;i<VPBUFFER_FRAMES*bin->chans;i++){
            float tmp=buffer[i]*sb_preamp;
            if (tmp > 0.95f && sb_preamp > 32.0f) {
                sb_preamp*=0.999f;
                tmp*=0.999f;
                ff=false;
            }
            if (tmp > 0.2f || tmp < -0.2f) {
                xx=false;
            }
            buffer[i]=tmp;

			buffer[i]=CLIP(buffer[i]);

        }
        if (ff && xx && sb_preamp < 64.0f) {
            sb_preamp*=1.01f;
        }

    } else {
        for (register int i=0;i<VPBUFFER_FRAMES*bin->chans;i++){
            buffer[i]*=sb_preamp;
			buffer[i]=CLIP(buffer[i]);

        }
    }


/*
    float mid,xre,xim,newb;
    unsigned step=0,chans=2,ichans;
    float *bar_array_w=bar_array;
    float delta;
    float changes=0.0f;

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
            newb = (fabs(xre)+fabs(xim))*(0.6f+ b*1.7f);
            bar_array_w[b] = (newb<limit)?newb:limit;
            if (bar_array_w[b] > mids[b]){
                mids[b]=mids[b]*0.8f+bar_array_w[b]*0.2f;
            } else {
                mids[b]=mids[b]*0.99f;
            }

        }

        for (unsigned b=0;b<BAR_COUNT;b++){
            for (unsigned h=0;h<10;h++){
                delta = 0.0002f*(mids[b]*knowledge[b]-target[b])*mids[b];
                knowledge[b] -= delta;
                changes+=delta;

            }
        }

        step+=1;
        bar_array_w += BAR_COUNT;
        buffer += VPBUFFER_PERIOD*2;
    }
    if (changes > 0.002f*BAR_SETS && !period_count){
        applyKnowledge();
    }
    period_count = (period_count + BAR_SETS) % 100;
*/
}

int VPEffectPluginEQ::finit()
{
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        VSettings::getSingleton()->writefloat(band,target[i]);
    }


    VSettings::getSingleton()->writefloat("eqpre",sb_preamp);

    equ_quit(&sb_state);
    memset(&sb_state, 0, sizeof(SuperEqState));

    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = NULL;

    initd = false;

    return 0;
}
