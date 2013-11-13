/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "vputils.h"
#include "eq.h"

VPEffectPluginEQ::VPEffectPluginEQ(float cap)
{
    sb_preamp = (float) VSettings::getSingleton()->readfloat("eqpre",64.0f);
    sb_paramsroot = NULL;
    sched_recalc = false;
    owner=NULL;
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        target[i]=VSettings::getSingleton()->readfloat(band,pow(10,48.0f/-20));
    }
    memset(&sb_state, 0, sizeof(SuperEqState));

    bar_array = NULL;
    for (size_t i=0;i<BAR_COUNT;i++){
        trig[0][i]=(float *)new float[VPBUFFER_PERIOD*sizeof(float)];
        trig[1][i]=(float *)new float[VPBUFFER_PERIOD*sizeof(float)];
    }

    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eqk");
        band.append(std::to_string(i));
        knowledge[i]=VSettings::getSingleton()->readfloat(band,1.0f);
    }

    limit= cap;
    initd=false;

    period_count = 0;
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

    for (size_t i=0;i<BAR_COUNT;i++){
        delete[] trig[0][i];
        delete[] trig[1][i];
    }
}
void VPEffectPluginEQ::sb_recalc_table()
{
    void *params = paramlist_alloc ();

    float bands_copy[BAR_COUNT];
    memcpy (bands_copy, sb_bands, sizeof (sb_bands));
    for (int i = 0; i < BAR_COUNT; i++) {
        bands_copy[i] *= sb_preamp;
    }

    equ_makeTable (&sb_state, bands_copy, params, bin->srate);
    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = params;

}

int VPEffectPluginEQ::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{

    if (bar_array)
        delete[] bar_array;
    bar_array = new float[BAR_COUNT*BAR_SETS];

    for (size_t i=0;i<BAR_COUNT*BAR_SETS;i++){
        bar_array[i] = 0.0f;
    }
    for (unsigned i=0;i<BAR_COUNT;i++){
        mids[i] = 20.0f;
        sb_bands[i] = target[i];
    }

    owner = v;
    bin = in;
    bout = in;
    *out = in;

    float fn=0.0f, fi;
    for (size_t b=0;b<BAR_COUNT;b++){
        // lets just ASSUME that our buffer frame count is 44100

        fn = (2*freqs[b])/(bin->srate);
        freq_p[b]=fn;

        fi = PI*fn;
        for (size_t i=0;i<VPBUFFER_PERIOD;i++){
            trig[0][b][i]=cosf(fi*i);
            trig[1][b][i]=sinf(fi*i);
        }
    }

    equ_init (&sb_state, 14, bin->chans);
    sb_recalc_table();
    initd = true;

    return 0;

}
void VPEffectPluginEQ::applyKnowledge()
{

    for (unsigned i=0;i<BAR_COUNT;i++){
        float next=knowledge[i]*mids[i]*0.6f + sb_bands[i]*0.4f;
        if (next<target[i] && next>target[i]-3.0f){
            sb_bands[i] = next;
        } else {
            knowledge[i]*=0.5f;
        }
    }
    sched_recalc=true;

}
void VPEffectPluginEQ::process(float *buffer)
{
    if (sched_recalc){
        sb_recalc_table();
        sched_recalc=false;
    }

    //assert(buffer == bin->buffer);

    equ_modifySamples_float(&sb_state, (char *)buffer, VPBUFFER_FRAMES, bin->chans);
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
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eqk");
        band.append(std::to_string(i));
        VSettings::getSingleton()->writefloat(band,knowledge[i]);
    }
    VSettings::getSingleton()->writefloat("eqpre",sb_preamp);

    equ_quit(&sb_state);
    memset(&sb_state, 0, sizeof(SuperEqState));

    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = NULL;

    if (bar_array)
        delete[] bar_array;
    bar_array = NULL;

    initd = false;

    return 0;
}
