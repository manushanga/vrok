#ifndef CONFIG_OUT_H
#define CONFIG_OUT_H

#include <map>
#include <list>
#include <cassert>

#define VPBUFFER_PERIOD 512

#include "vputils.h"
#include "out.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"

#include "cpplib.h"

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
    void writeInt(std::string field, int i);
    void writeDouble(std::string field, double dbl);
    void writefloat(std::string field, float flt);
    void writeString(std::string field, std::string str);
    int readInt(std::string field, int def);
    double readDouble(std::string field, double def);
    float readfloat(std::string field, float def);
    std::string readString(std::string field, std::string def);
    ~VSettings();
};

class VPOutFactory
{
private:
    std::map<std::string, vpout_entry_t> creators;
    std::string currentOut;
public:
    static VPOutFactory *getSingleton()
    {
        static VPOutFactory vpf;
        return &vpf;
    }
    inline uint getBufferSize()
    {
        std::map<std::string, vpout_entry_t>::iterator it=creators.find(currentOut);
        if (it == creators.end()) {
            DBG("Error getting buffer size for outplugin");
            return 0;
        } else {
            return it->second.frames;
        }
    }
    VPOutFactory();
    inline VPOutPlugin *create()
    {
        std::map<std::string, vpout_entry_t>::iterator it=creators.find(currentOut);
        if (it!= creators.end()){
            return (VPOutPlugin *)it->second.creator();
        } else {
            return NULL;
        }
    }

    inline VPOutPlugin *create(std::string name)
    {
        std::map<std::string, vpout_entry_t>::iterator it=creators.find(name);
        if (it!= creators.end()){
            currentOut = name;
            VSettings::getSingleton()->writeString("outplugin",currentOut);
            return (VPOutPlugin *)it->second.creator();
        } else {
            return NULL;
        }
    }

};

struct vpdecoder_entry_t{
    std::string name;
    vpdecode_creator_t creator;
};

class VPDecoderFactory{
private:
    std::map<std::string, vpdecoder_entry_t> creators;
public:
    static VPDecoderFactory *getSingleton()
    {
        static VPDecoderFactory vpd;
        return &vpd;
    }
    VPDecoderFactory();
    VPDecoderPlugin *create(std::string ext, VPlayer *v)
    {
        std::map<std::string, vpdecoder_entry_t>::iterator it=creators.find(ext);
        if (it == creators.end())
        {
            WARN("no decoder for "<<ext);
            return NULL;
        } else {
            return (VPDecoderPlugin *)it->second.creator(v);
        }
    }
    int count()
    {
        return (int)creators.size();
    }
    void getExtensionsList(std::vector<std::string>& list)
    {
        for (std::map<std::string, vpdecoder_entry_t>::iterator it=creators.begin();
             it!=creators.end();
             it++)
        {
            list.push_back(it->first);
        }
    }
};
#define VPBUFFER_FRAMES VPOutFactory::getSingleton()->getBufferSize()
#endif // CONFIG_OUT_H

