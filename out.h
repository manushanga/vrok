#ifndef OUT_H
#define OUT_H

#include <stdint.h>

#include "out_jack.h"

#define VPOUT_BUFFER_SAMPLES 4096

typedef struct _vpout_plugin_t{
    char vpout_plugin_name[64];
    int (*vpout_plugin_init)(char *name);
    void (*vpout_plugin_run)(float *samples, unsigned count);
    unsigned (*vpout_plugin_samplerate)();
    unsigned (*vpout_plugin_channels)();
    int (*vpout_plugin_end)();
}vpout_plugin_t;

int vpout_init(char *portname, unsigned op);
void vpout_run(void *data, const void *samples, unsigned count, int64_t pts);
int vpout_end();
vpout_plugin_t *vpout_plugin_get(unsigned op);
#endif // OUT_H
