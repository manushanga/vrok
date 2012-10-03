#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "vputils.h"
#include "out.h"
#include "shibatch/Equ.h"

static vpout_plugin_t vpout_plugins[] =
{ {"Jack Audio Connection Kit", jack_init,jack_run,jack_samplerate,jack_channels,jack_end}
};

static vpout_plugin_t *current=NULL;
static unsigned vpout_channels=0;
static unsigned vpout_samplerate=0;
float *vpout_buffer=NULL;

static const char *sb_bandnames[] = {
    "Preamp",
    "55 Hz",
    "77 Hz",
    "110 Hz",
    "156 Hz",
    "220 Hz",
    "311 Hz",
    "440 Hz",
    "622 Hz",
    "880 Hz",
    "1.2 kHz",
    "1.8 kHz",
    "2.5 kHz",
    "3.5 kHz",
    "5 kHz",
    "7 kHz",
    "10 kHz",
    "14 kHz",
    "20 kHz"
};
SuperEqState sb_state;
float sb_bands[18]= {2.2f,2.5f,2.3f,1.2f,1.1f,1.0f,
                     0.7f,0.6f,0.5f,0.5f,0.6f,0.7f,
                     1.0f,1.1f,1.2f,1.3f,2.3f,2.2f};
float sb_preamp;
void *sb_paramsroot;

static void sb_recalc_table ()
{
    void *params = paramlist_alloc ();

    float bands_copy[18];
    memcpy (bands_copy, sb_bands, sizeof (sb_bands));
    for (int i = 0; i < 18; i++) {
        bands_copy[i] *= sb_preamp;
    }

    equ_makeTable (&sb_state, bands_copy, params, vpout_samplerate);

    paramlist_free (sb_paramsroot);
    sb_paramsroot = params;
}

vpout_plugin_t *vpout_plugin_get(unsigned op){
    DBG("Jack::plug"<<vpout_plugins[op].vpout_plugin_name);
    return &vpout_plugins[op];
}

int vpout_init(char *portname, unsigned op){
    current = &vpout_plugins[op];
    current->vpout_plugin_init(portname);
    vpout_channels = current->vpout_plugin_channels();
    vpout_samplerate = current->vpout_plugin_samplerate();

    vpout_buffer = (float *) malloc(VPOUT_BUFFER_SAMPLES*sizeof(float)*current->vpout_plugin_channels());
    equ_init (&sb_state, 10, vpout_channels);
    sb_preamp = 1.0f;
    //sb_bands
    //for (int i = 0; i < 18; i++) {
    //   sb_bands[i] = 1.0f;
    //}
    sb_recalc_table();
    return 0;
}

void vpout_run(void *data, const void *samples, unsigned count, int64_t pts)
{
    short *s_samples= (short *) samples;
    // VP_SAMPLES > count
    if (VPOUT_BUFFER_SAMPLES < count) {
        DBG("VPOUT::vpout_buffer not enough");
        exit(1);
    }

    for (unsigned i=0;i<count*vpout_channels;i++){
        vpout_buffer[i] = s_samples[i]/32767.0f;
    }
    equ_modifySamples_float(&sb_state, (char *)vpout_buffer, count, vpout_channels);
    current->vpout_plugin_run(vpout_buffer, count);
}

int vpout_end()
{
    current->vpout_plugin_end();
    free(vpout_buffer);
    return 0;
}
