#include "vrok.h"

#if defined(VPOUT_DUMMY)
#include "outs/dummy.h"
#elif defined(VPOUT_DSOUND)
#include "outs/dsound.h"
#elif defined(VPOUT_ALSA)
#include "outs/alsa.h"
#elif defined(VPOUT_AO)
#include "outs/ao.h"
#elif defined(VPOUT_PULSE)
#include "outs/pulse.h"
#endif

VPOutFactory::VPOutFactory()
{

    // NOTE: Keep buffer sizes 2^n for FFT
    // TODO: Resolve visualization needing PERIOD*4 frames better to have full
    //       freedom here
#if defined(VPOUT_DUMMY)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginDummy::VPOutPluginDummy_new, VPBUFFER_PERIOD*4 };
    creators.insert(std::pair<std::string, vpout_entry_t> ("Dummy",def));
    currentOut = "Dummy";
#elif defined(VPOUT_DSOUND)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginDSound::VPOutPluginDSound_new, VPBUFFER_PERIOD*16 };
    creators.insert(std::pair<std::string, vpout_entry_t> ("DSound",def));
    currentOut = "DSound";
#elif defined(VPOUT_ALSA)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginAlsa::VPOutPluginAlsa_new, VPBUFFER_PERIOD*4 };
    creators.insert(std::pair<std::string, vpout_entry_t> ("ALSA",def));
    currentOut = "ALSA";
#elif defined(VPOUT_PULSE)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginPulse::VPOutPluginPulse_new, VPBUFFER_PERIOD*4 };
    creators.insert(std::pair<std::string, vpout_entry_t> ("PulseAudio",def));
    currentOut = "PulseAudio";
#elif defined(VPOUT_AO)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginAO::VPOutPluginAO_new, VPBUFFER_PERIOD*4 };
    creators.insert(std::pair<std::string, vpout_entry_t> ("AO",def));
    currentOut = "AO";
#endif

    currentOut=VSettings::getSingleton()->readString("outplugin",currentOut);

}

VPDecoderFactory::VPDecoderFactory()
{
    vpdecoder_entry_t decoders[]= {
     {"FLAC", (vpdecode_creator_t)FLACDecoder::VPDecoderFLAC_new, "file", "flac" },
     {"MPEG",(vpdecode_creator_t)MPEGDecoder::VPDecoderMPEG_new, "file", "mpX,mp2,mp1"},
     {"OGG",(vpdecode_creator_t)OGGDecoder::VPDecoderOGG_new, "file", "ogg"},
     {"FFMPEG",(vpdecode_creator_t)FFMPEGDecoder::VPDecoderFFMPEG_new, "ANY", "ANY"}
    };

    for (int i=0;i<sizeof(decoders)/sizeof(vpdecoder_entry_t);i++){
        decoders_.push_back(decoders[i]);
    }
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("flac",decoders[0]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp1",decoders[1]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp2",decoders[1]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp3",decoders[1]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("ogg",decoders[2]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mpg",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("avi",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mpeg",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp4",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mkv",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("dat",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("vob",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("aac",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("wav",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("flv",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("wma",decoders[3]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("wmv",decoders[3]));

}


VPDecoderPlugin *VPDecoderFactory::create(VPResource& resource, VPlayer *v)
{
    for (std::vector< vpdecoder_entry_t >::iterator it=decoders_.begin();it!=decoders_.end();it++)
    {
        DBG(it->name<<" "<<resource.getExtension()<<" "<<resource.getProtocol());
        std::string ext,proto;
        ext=resource.getExtension();
        proto=resource.getProtocol();

        if (it->protocols.compare("ANY") == 0){
            if (it->extensions.compare("ANY") == 0){
                return (VPDecoderPlugin *) it->creator(v);
            } else if (it->extensions.find(ext) != std::string::npos) {
                return (VPDecoderPlugin *) it->creator(v);
            }
        } else if (it->protocols.find(proto) != std::string::npos) {
            if (it->extensions.compare("ANY") == 0){
                return (VPDecoderPlugin *) it->creator(v);
            } else if (it->extensions.find(ext) != std::string::npos) {
                return (VPDecoderPlugin *) it->creator(v);
            }
        }
    }
    WARN("No decoder found");
    return NULL;
}
int VPDecoderFactory::count()
{
    return (int)decoders_.size();
}
void VPDecoderFactory::getExtensionsList(std::vector<std::string>& list)
{
    for (std::map<std::string, vpdecoder_entry_t>::iterator it=creators.begin();
         it!=creators.end();
         it++)
    {
        list.push_back(it->first);
    }
}
