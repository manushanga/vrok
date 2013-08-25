#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>
class VPlayer;
typedef void *(*vpdecode_creator_t)(VPlayer *v);

typedef struct _vpdecoder_entry{
    char name[32];
    char ext[8];
    vpdecode_creator_t creator;
}vpdecoder_entry_t;


#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
static vpdecoder_entry_t vpdecoder_entries[] = {
                                                 {"FLAC", "flac" ,(vpdecode_creator_t)FLACDecoder::VPDecoderFLAC_new },
                                                 {"MPEG", "mp1" ,(vpdecode_creator_t)MPEGDecoder::VPDecoderMPEG_new },
                                                 {"MPEG", "mp2" ,(vpdecode_creator_t)MPEGDecoder::VPDecoderMPEG_new },
                                                 {"MPEG", "mp3" ,(vpdecode_creator_t)MPEGDecoder::VPDecoderMPEG_new },
                                                 {"OGG", "ogg" ,(vpdecode_creator_t)OGGDecoder::VPDecoderOGG_new }
                                               };
class VSettings {
private:
    std::map<std::string, std::vector<int> > settings;
public:
    static VSettings *getSingleton();

    VSettings();
    void writeInt(std::string field, int i);
    void writeDouble(std::string field, double dbl);
    void writeFloat(std::string field, float flt);
    void writeString(std::string field, std::string str);
    int readInt(std::string field, int def);
    double readDouble(std::string field, double def);
    float readFloat(std::string field, float def);
    std::string readString(std::string field, std::string def);
    ~VSettings();
};


#endif // CONFIG_H
