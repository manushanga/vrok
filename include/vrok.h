#ifndef CONFIG_OUT_H
#define CONFIG_OUT_H

#include <cassert>

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

    static const vpout_entry_t vpout_entries[] = { {"DSound", (vpout_creator_t)VPOutPluginDSound::VPOutPluginDSound_new, VPBUFFER_PERIOD*6 } };
#elif defined(__linux__)
    #ifdef VPOUT_ALSA
        #include "outs/alsa.h"
    #elif defined(VPOUT_PULSE)
        #include "outs/pulse.h"
    #endif
    static const vpout_entry_t vpout_entries[] = {
    #ifdef VPOUT_ALSA
          {"ALSA", (vpout_creator_t)VPOutPluginAlsa::VPOutPluginAlsa_new, VPBUFFER_PERIOD }
    #elif defined(VPOUT_PULSE)
          {"PULSE", (vpout_creator_t)VPOutPluginPulse::VPOutPluginPulse_new, VPBUFFER_PERIOD }
    #endif
    };

#else
    #error "Unsupported platform."
#endif

#define VPBUFFER_FRAMES vpout_entries[DEFAULT_VPOUT_PLUGIN].frames

#include "vputils.h"
#endif // CONFIG_OUT_H

