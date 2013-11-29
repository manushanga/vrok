/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "vputils.h"
#include "eq.h"
#define CONV(x,y) (x+0.8*y)/1.8
#define WRAPINC(x,max) (x+1) % max
VPEffectPluginEQ::VPEffectPluginEQ(float cap)
{
    sb_preamp = (float) VSettings::getSingleton()->readfloat("eqpre",96.0f);
    autopreamp = (bool) VSettings::getSingleton()->readInt("autopreamp",0);
    sb_paramsroot = NULL;
    sched_recalc = false;
    owner=NULL;
    for (int i=0;i<BAR_COUNT;i++) {
        std::string band("eq");
        band.append(std::to_string(i));
        target[i]=VSettings::getSingleton()->readfloat(band,pow(10,48.0f/-20));
    }
    memset(&sb_state, 0, sizeof(SuperEqState));
    limit= cap;
    initd=false;
    reverb_length=VPBUFFER_FRAMES;
    reverb_start=VPBUFFER_FRAMES;
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

    reverb_buffer = (float*) ALIGNED_ALLOC(VPBUFFER_FRAMES*bin->chans*sizeof(float)*2);

    for (int i=0;i<VPBUFFER_FRAMES*bin->chans*2;i++) { reverb_buffer[i] = 0.0f; }

    equ_init (&sb_state, 14, bin->chans);
    sb_recalc_table();
    initd = true;

    reverb_read = 0;
    reverb_read_ = 200*bin->chans;
    reverb_write = 0;
    reverb_buffer_filled=false;
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
    register int maxx=VPBUFFER_FRAMES*bin->chans*2;

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
        }
        if (ff && xx && sb_preamp < 64.0f) {
            sb_preamp*=1.01f;
        }
    } else {
        for (register int i=0;i<VPBUFFER_FRAMES*bin->chans;i++){
            buffer[i]*=sb_preamp;
        }
    }


    for (register int i=0;i<VPBUFFER_FRAMES*bin->chans;i++) {
        reverb_buffer[reverb_write]=buffer[i];
        reverb_write=WRAPINC(reverb_write,maxx);
        if (reverb_write == maxx - 1)
            reverb_buffer_filled = true;
    }
    if (!reverb_buffer_filled) {

        for (register int i=0;i<VPBUFFER_FRAMES*bin->chans;i++) {
            buffer[i]=0.0f;
        }
    } else {
        for (register int i=0;i<VPBUFFER_FRAMES*bin->chans;i++) {

            buffer[i]=CONV(reverb_buffer[reverb_read],reverb_buffer[(reverb_read+1000)%maxx]);
            buffer[i]=CONV(buffer[i],reverb_buffer[(reverb_read+500) % maxx]);
            reverb_read=WRAPINC(reverb_read,maxx);
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

    ALIGNED_FREE(reverb_buffer);

    VSettings::getSingleton()->writefloat("eqpre",sb_preamp);

    equ_quit(&sb_state);
    memset(&sb_state, 0, sizeof(SuperEqState));

    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = NULL;

    initd = false;

    return 0;
}
