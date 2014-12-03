/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "vputils.h"
#include "eq.h"

VPEffectPluginEQ::VPEffectPluginEQ()
{
    sb_preamp = (float) VSettings::getSingleton()->readFloat("eqpre",1.0f);
    autopreamp = (bool) VSettings::getSingleton()->readInt("autopreamp",0);
    sb_paramsroot = NULL;
    sched_recalc = true;

    owner=NULL;
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        sb_bands[i]=VSettings::getSingleton()->readFloat(band,1.0f);

    }
    memset(&sb_state, 0, sizeof(SuperEqState));
    initd=false;

}

VPEffectPluginEQ::~VPEffectPluginEQ()
{
    if (initd)
        finit();
}

void VPEffectPluginEQ::sb_recalc_table()
{
    void *params = paramlist_alloc ();
    float sb_bands_copy[18];
    for (int i=0;i<BAR_COUNT;i++){
        sb_bands_copy[i]=sb_bands[i]*sb_preamp;
    }

    equ_makeTable (&sb_state, sb_bands_copy, params, bin->srate);
    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = params;

}

int VPEffectPluginEQ::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{
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
    int samples_per_chan = VPBUFFER_FRAMES;
    int samples = VPBUFFER_FRAMES*bin->chans;

    if (sched_recalc){
        sb_recalc_table();
        ATOMIC_CAS(&sched_recalc,true,false);
    }

    //assert(buffer == bin->buffer);

    equ_modifySamples_float(&sb_state, (char *)buffer, samples_per_chan, bin->chans);


}

int VPEffectPluginEQ::finit()
{
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        VSettings::getSingleton()->writeFloat(band,sb_bands[i]);
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
