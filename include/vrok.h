#ifndef CONFIG_OUT_H
#define CONFIG_OUT_H

#include <map>
#include <set>
#include <list>
#include <cassert>

#define VPBUFFER_PERIOD 512

#include "vputils.h"
#include "out.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "players/ffmpeg.h"

#include "cpplib.h"

#define VERSION 4

class VPlayer;

typedef void *(*vpout_creator_t)(void);
typedef void *(*vpdecode_creator_t)(VPlayer *v);



struct vpout_entry_t{
    vpout_creator_t creator;
    unsigned frames;
};

class VSettings {
private:
    std::map<std::string, std::vector<int> > settings;
public:
    static VSettings *getSingleton();

    VSettings();
    std::string getSettingsPath();
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

class VPOutFactory
{
private:
    std::map<std::string, vpout_entry_t> creators;
    std::string currentOut;
public:
    static VPOutFactory *getSingleton();
    VPOutFactory();
    int getBufferSize();
    VPOutPlugin *create();
    VPOutPlugin *create(std::string name);

};

struct vpdecoder_entry_t{
    std::string name;
    vpdecode_creator_t creator;
    std::string protocols;
    std::string extensions;
};

class VPDecoderFactory{
private:
    std::vector< vpdecoder_entry_t > decoders_;
public:
    static VPDecoderFactory *getSingleton();
    VPDecoderFactory();
    VPDecoderPlugin *create(VPResource& resource, VPlayer *v);
    int count();
    void getExtensionsList(std::vector<std::string>& list);
};
#define VPBUFFER_FRAMES VPOutFactory::getSingleton()->getBufferSize()
#endif // CONFIG_OUT_H

