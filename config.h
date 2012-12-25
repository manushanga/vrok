#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

#include "out.h"
typedef void *(*creator_t)(void);

typedef struct _vpout_entry{
    char name[32];
    creator_t creator;
}vpout_entry_t;

typedef struct _vpdecoder_entry{
    char name[32];
    char ext[8];
    creator_t creator;
}vpdecoder_entry_t;


#ifdef _WIN32
    #include "outs/waveout.h"
    #define DEFAULT_VPOUT_PLUGIN "WaveOut"

    static vpout_entry_t vpout_entries[] = { {"WaveOut", (creator_t)_VPOutPluginWaveOut_new } };
#elif defined(__linux__)
    #include "outs/alsa.h"
    #define DEFAULT_VPOUT_PLUGIN "ALSA"

    static vpout_entry_t vpout_entries[] = { {"ALSA", (creator_t)_VPOutPluginAlsa_new } };
#else
    #error "Unsupported platform."
#endif

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
static vpdecoder_entry_t vpdecoder_entries[] = {
                                                 {"FLAC", "flac" ,(creator_t)_VPDecoderFLAC_new },
                                                 {"MPEG", "mp1" ,(creator_t)_VPDecoderMPEG_new },
                                                 {"MPEG", "mp2" ,(creator_t)_VPDecoderMPEG_new },
                                                 {"MPEG", "mp3" ,(creator_t)_VPDecoderMPEG_new },
                                                 {"OGG", "ogg" ,(creator_t)_VPDecoderOgg_new }
                                               };

void config_init();
creator_t config_get_VPOutPlugin_creator();
void config_finit();

#endif // CONFIG_H
