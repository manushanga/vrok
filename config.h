#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

#include "out.h"
typedef void *(*creator_t)(void);

typedef struct _vpout_entry{
    char name[32];
    creator_t creator;
}vpout_entry_t;

#ifdef _WIN32
    #include "outs/waveout.h"
    #define DEFAULT_VPOUT_PLUGIN "WaveOut"

    static vpout_entry_t vpout_entries[] = { {"WaveOut", (creator_t)_VPOutPluginWaveOut_new) } };
#elif defined(__linux__)
    #include "outs/alsa.h"
    #define DEFAULT_VPOUT_PLUGIN "ALSA"

    static vpout_entry_t vpout_entries[] = { {"ALSA", (creator_t)_VPOutPluginAlsa_new } };
#else
    #error "Unsupported platform."
#endif

void config_init();
creator_t config_get_VPOutPlugin_creator();
void config_finit();

#endif // CONFIG_H
