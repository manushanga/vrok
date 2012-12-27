/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>

#include "eq.h"

static const char *sb_bandnames[] = { "Preamp", "55 Hz", "77 Hz", "110 Hz",
    "156 Hz", "220 Hz", "311 Hz", "440 Hz", "622 Hz", "880 Hz", "1.2 kHz",
    "1.8 kHz", "2.5 kHz", "3.5 kHz", "5 kHz", "7 kHz", "10 kHz", "14 kHz",
    "20 kHz"
};
static float sb_bands[18]= {0.6f,0.8f,0.55f,0.45f,0.4f,0.3f,
                            0.2f,0.1f,0.1f,0.1f,0.2f,0.3f,
                            0.3f,0.35f,0.4f,0.5f,0.55f,0.3f};

void VPEffectPluginEQ::sb_recalc_table()
{
   // this->owner->mutexes[0].lock();
   // this->owner->mutexes[2].lock();

    void *params = paramlist_alloc ();

    float bands_copy[18];
    memcpy (bands_copy, sb_bands, sizeof (sb_bands));
    for (int i = 0; i < 18; i++) {
        bands_copy[i] *= sb_preamp;
    }

    equ_makeTable (&sb_state, bands_copy, params, owner->track_samplerate);

    paramlist_free (sb_paramsroot);
    sb_paramsroot = params;

    //this->owner->mutexes[0].unlock();
    //this->owner->mutexes[2].unlock();

}
int VPEffectPluginEQ::init(VPlayer *v)
{
    owner = v;
    equ_init (&sb_state, 10, owner->track_channels);
    sb_preamp = 2.3f;
    sb_recalc_table();
    return 1;

}

void VPEffectPluginEQ::process(float *buffer)
{
    equ_modifySamples_float(&sb_state, (char *)buffer, owner->BUFFER_FRAMES, owner->track_channels);
}

int VPEffectPluginEQ::finit()
{
    equ_quit(&sb_state);
    return 1;
}
