#include <stdlib.h>
#include <unistd.h>

#include "vputils.h"
#include "out.h"

static vpout_plugin_t vpout_plugins[] =
{ {"Jack Audio Connection Kit", jack_init,jack_run,jack_samplerate,jack_channels,jack_end}
};

static vpout_plugin_t *current=NULL;
static unsigned vpout_channels=0;
float *vpout_buffer=NULL;

vpout_plugin_t *vpout_plugin_get(unsigned op){
    DBG("Jack::plug"<<vpout_plugins[op].vpout_plugin_name);
    return &vpout_plugins[op];
}

int vpout_init(char *portname, unsigned op){
    current = &vpout_plugins[op];
    current->vpout_plugin_init(portname);
    vpout_channels = current->vpout_plugin_channels();
    vpout_buffer = (float *) malloc(VPOUT_BUFFER_SAMPLES*sizeof(float)*current->vpout_plugin_channels());
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
        vpout_buffer[i] = s_samples[i]/5000.0f;
    }
    current->vpout_plugin_run(vpout_buffer, count);
}

int vpout_end()
{
    current->vpout_plugin_end();
    free(vpout_buffer);
    return 0;
}
