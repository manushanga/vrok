#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

typedef void *(*vpdecode_creator_t)(void);

typedef struct _vpdecoder_entry{
    char name[32];
    char ext[8];
    vpdecode_creator_t creator;
}vpdecoder_entry_t;


#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
static vpdecoder_entry_t vpdecoder_entries[] = {
                                                 {"FLAC", "flac" ,(vpdecode_creator_t)_VPDecoderFLAC_new },
                                                 {"MPEG", "mp1" ,(vpdecode_creator_t)_VPDecoderMPEG_new },
                                                 {"MPEG", "mp2" ,(vpdecode_creator_t)_VPDecoderMPEG_new },
                                                 {"MPEG", "mp3" ,(vpdecode_creator_t)_VPDecoderMPEG_new },
                                                 {"OGG", "ogg" ,(vpdecode_creator_t)_VPDecoderOgg_new }
                                               };

void config_init();
float config_get_volume();
QString config_get_lastopen();
void config_set_lastopen(QString last);
void config_set_volume();
void config_finit();

#endif // CONFIG_H
