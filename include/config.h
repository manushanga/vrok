#ifndef CONFIG_H
#define CONFIG_H

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

#include <QString>

void config_init();

void config_set_lastopen(QString last);
void config_set_eq_preamp(float pa);
void config_set_eq_bands(float *bands);
void config_set_eq_knowledge_bands(float *bands);
void config_set_volume();

QString config_get_lastopen();
float config_get_eq_preamp();
void config_get_eq_bands(float *bands);
void config_get_eq_knowledge_bands(float *bands);
float config_get_volume();

void config_finit();

#endif // CONFIG_H
