#ifndef CONFIG_OUT_H
#define CONFIG_OUT_H

typedef void *(*vpout_creator_t)(void);

#define VPBUFFER_PERIOD 512

typedef struct _vpout_entry{
    char name[8];
    vpout_creator_t creator;
    unsigned frames;
}vpout_entry_t;

#define DEFAULT_VPOUT_PLUGIN 0
#ifdef _WIN32
    #include "outs/dsound.h"

    static const vpout_entry_t vpout_entries[] = { {"DSound", (vpout_creator_t)_VPOutPluginDSound_new, VPBUFFER_PERIOD*6 } };
#elif defined(__linux__)
    #include "outs/alsa.h"

    static const vpout_entry_t vpout_entries[] = { {"ALSA", (vpout_creator_t)_VPOutPluginAlsa_new, VPBUFFER_PERIOD } };
#else
    #error "Unsupported platform."
#endif

#define VPBUFFER_FRAMES vpout_entries[DEFAULT_VPOUT_PLUGIN].frames

#endif // CONFIG_OUT_H

