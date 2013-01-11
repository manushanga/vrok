/*
  Vrok - smokin' audio
  (C) 20.0f12 Madura A. released under GPL 2.0.0f. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "config_out.h"
#include "config.h"
#include "eq.h"

static const char *sb_bandnames[] = { "55 Hz", "77 Hz", "110 Hz",
    "156 Hz", "220 Hz", "311 Hz", "440 Hz", "622 Hz", "880 Hz", "1.2 kHz",
    "1.8 kHz", "2.5 kHz", "3.5 kHz", "5 kHz", "7 kHz", "10 kHz", "14 kHz",
    "20 kHz"
};
VPEffectPluginEQ::VPEffectPluginEQ()
{
    sb_preamp = config_get_eq_preamp();
    sb_paramsroot = NULL;
    owner=NULL;
    config_get_eq_bands(sb_bands);

    memset(&sb_state, 0, sizeof(SuperEqState));
}
VPEffectPluginEQ::~VPEffectPluginEQ()
{

}
float VPEffectPluginEQ::getPreamp()
{
    return sb_preamp;
}
void VPEffectPluginEQ::sb_recalc_table()
{
    void *params = paramlist_alloc ();

    float bands_copy[18];
    memcpy (bands_copy, sb_bands, sizeof (sb_bands));
    for (int i = 0; i < 18; i++) {
        bands_copy[i] *= sb_preamp;
    }

    equ_makeTable (&sb_state, bands_copy, params, (owner->track_samplerate==0?44100:owner->track_samplerate));
    if (sb_paramsroot)
        paramlist_free (sb_paramsroot);
    sb_paramsroot = params;

}
const char **VPEffectPluginEQ::getBandNames()
{
    return sb_bandnames;
}
void VPEffectPluginEQ::setBand(int i, float val)
{
    sb_bands[i]=val;
    if (owner)
        sb_recalc_table();
}
void VPEffectPluginEQ::setPreamp(float val)
{
    sb_preamp = val;
    if (owner)
        sb_recalc_table();
}
float *VPEffectPluginEQ::getBands()
{
    return sb_bands;
}
int VPEffectPluginEQ::init(VPlayer *v)
{
    owner = v;
    equ_init (&sb_state, 10.0f, owner->track_channels);
    sb_recalc_table();
    work=false;
    return 0;

}

void VPEffectPluginEQ::process(float *buffer)
{
    work=true;
    equ_modifySamples_float(&sb_state, (char *)buffer, VPBUFFER_FRAMES, owner->track_channels);
    work=false;
}

int VPEffectPluginEQ::finit()
{
    while (work){};
    equ_quit(&sb_state);
    memset(&sb_state, 0, sizeof(SuperEqState));
    config_set_eq_bands(sb_bands);
    config_set_eq_preamp(sb_preamp);
    return 0;
}
