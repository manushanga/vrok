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
VPEffectPluginEQ::VPEffectPluginEQ()
{
    sb_preamp = (float) VSettings::getSingleton()->readFloat("eqpre",96.0f);
    autopreamp = (bool) VSettings::getSingleton()->readInt("autopreamp",0);
    sb_paramsroot = NULL;
    sched_recalc = true;

    owner=NULL;
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        target[i]=VSettings::getSingleton()->readFloat(band,pow(10,48.0f/-20));
        DBG(band<<" "<<target[i]);
    }
    memset(&sb_state, 0, sizeof(SuperEqState));
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

void VPEffectPluginEQ::process()
{
    float *buffer = bin->currentBuffer();
    int samples_per_chan = *bin->currentBufferSamples();
    int samples = samples_per_chan*bin->chans;

    if (sched_recalc){
        sb_recalc_table();
        ATOMIC_CAS(&sched_recalc,true,false);
    }

    //assert(buffer == bin->buffer);

    equ_modifySamples_float(&sb_state, (char *)buffer, samples_per_chan, bin->chans);


    if (autopreamp) {
        bool ff=true, xx=true;
        for (register int i=0;i<samples;i++){
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
        for (register int i=0;i<samples;i++){
            buffer[i]*=sb_preamp;
			buffer[i]=CLIP(buffer[i]);

        }
    }

}

int VPEffectPluginEQ::finit()
{
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        VSettings::getSingleton()->writeFloat(band,target[i]);
    }


    VSettings::getSingleton()->writeFloat("eqpre",sb_preamp);

    equ_quit(&sb_state);
    memset(&sb_state, 0, sizeof(SuperEqState));

    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = NULL;

    initd = false;

    return 0;
}
