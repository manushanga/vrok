/* JACK Audio Connection Kit Plugin
 *
 */
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/mman.h>

#include <jack/jack.h>

#include "out_jack.h"
#include "vputils.h"

jack_port_t *output_ports[8];
jack_client_t *client;

static float (*buffer)[JACK_SAMPLES]=NULL;
static unsigned buffer_size=0;
static unsigned buffer_write=0;
static unsigned buffer_read=0;
static unsigned buffer_channels=0;

static unsigned run_ch_samples=0;
static unsigned run_write=0;
static unsigned run_read=0;
static unsigned run_write_larger=1;

typedef jack_default_audio_sample_t jack_sample_t;

jack_sample_t *process_out[8];

int jack_process (jack_nframes_t nframes, void *arg)
{
    for (unsigned ch=0;ch<buffer_channels;ch++){
        process_out[ch] = (jack_sample_t *)jack_port_get_buffer (output_ports[ch], nframes);
    }
    if ( (run_write_larger && (run_write > run_read)) || (!run_write_larger && (run_write < run_read)) ){
        for (unsigned i=0;i<nframes;i++){
            if (run_read >= JACK_SAMPLES){
                run_read=0;
                run_write_larger = !run_write_larger;
            }
            for (unsigned ch=0;ch<buffer_channels;ch++){
                process_out[ch][i]=buffer[ch][run_read];
            }
            run_read++;
        }
    } else {
        for (unsigned i=0;i<nframes;i++){
            for (unsigned ch=0;ch<buffer_channels;ch++){
                process_out[ch][i]=0.0f;
            }
        }
        DBG("Jack::xrun");
    }
    return 0;
}

void jack_shutdown(void *arg)
{
}

int jack_init(char *name)
{
    const char **ports;
    jack_status_t status;
    if ((client = jack_client_open(name, JackNullOption, &status, NULL)) == 0) {
        DBG("Jack::Jack server not running?");
        return 1;
    }

    jack_set_process_callback (client, jack_process, 0);
    jack_on_shutdown (client, jack_shutdown, 0);

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
        DBG("Jack::No playback ports");
        return 1;
    }
    unsigned chs=0;
    while (ports[chs]!=NULL){
        chs++;
    }
    buffer_channels = chs;
    DBG("Jack::channels "<<chs);

    char portname[25];
    for (unsigned i=0;i<buffer_channels;i++){
        sprintf(portname,"output_%d",i);
        output_ports[i] = jack_port_register (client, portname, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        if (!output_ports[i]){
            DBG("Jack::Opening ports failed");
            return 1;
        }
    }

    buffer_size = JACK_SAMPLES*buffer_channels*sizeof(float);
    buffer = (float (*)[JACK_SAMPLES]) malloc(buffer_size);
    mlock(buffer, buffer_size);

    if (jack_activate (client)) {
        DBG("Jack::Can't activate jack");
        return 1;
    }

    for (unsigned ch=0;ch<chs;ch++){
        DBG("Jack::channel "<<ports[ch]);
        int ret = jack_connect (client, jack_port_name (output_ports[ch]), ports[ch]);
        if (ret == EEXIST) {
            DBG("Jack::Connection exists for "<<ports[ch]);
        } else if (ret != 0) {
            DBG("Jack::Connection failed for "<<ports[ch]);
            return 1;
        }
    }
    free(ports);

    return 0;
}

unsigned jack_samplerate()
{
    DBG("Jack::samplerate "<<jack_get_sample_rate(client));
    return jack_get_sample_rate(client);
}

unsigned jack_channels(){
    return 2;
}

void jack_run(float *samples, unsigned count)
{

    if (JACK_SAMPLES < count){
        DBG("buffer too small");
    }

    for (unsigned i=0;i<count;i++){
        if (run_write >= JACK_SAMPLES){
            run_write = 0;
            run_write_larger = !run_write_larger;
        }
        for (unsigned ch=0;ch<buffer_channels;ch++){
            buffer[ch][run_write] = samples[buffer_channels*i+ch];
        }
        run_write++;
    }

}

int jack_end()
{
    int ret = jack_client_close(client);
    ret += jack_deactivate(client);
    DBG("Jack::end "<<ret);
    free(buffer);
    return ret;
}
