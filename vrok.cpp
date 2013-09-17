#include "vrok.h"

#if defined(VPOUT_DUMMY)
#include "outs/dummy.h"
#elif defined(_WIN32)
#include "outs/dsound.h"
#elif defined(__linux__)
#if defined(VPOUT_ALSA)
#include "outs/alsa.h"
#elif defined(VPOUT_AO)
#include "outs/ao.h"
#elif defined(VPOUT_PULSE)
#include "outs/pulse.h"
#endif
#endif

VPOutFactory::VPOutFactory()
{
#if defined(VPOUT_DUMMY)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginDummy::VPOutPluginDummy_new, VPBUFFER_PERIOD };
    creators.insert(std::pair<std::string, vpout_entry_t> ("Dummy",def));
    currentOut = "Dummy";
#elif defined(_WIN32)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginDSound::VPOutPluginDSound_new, VPBUFFER_PERIOD*6 };
    creators.insert(std::pair<std::string, vpout_entry_t> ("DSound",def));
    currentOut = "DSound";
#elif defined(__linux__)

#if defined(VPOUT_ALSA)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginAlsa::VPOutPluginAlsa_new, VPBUFFER_PERIOD };
    creators.insert(std::pair<std::string, vpout_entry_t> ("ALSA",def));
    currentOut = "ALSA";
#elif defined(VPOUT_PULSE)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginPulse::VPOutPluginPulse_new, VPBUFFER_PERIOD };
    creators.insert(std::pair<std::string, vpout_entry_t> ("PulseAudio",def));
    currentOut = "PulseAudio";
#elif defined(VPOUT_AO)
    vpout_entry_t def=
    { (vpout_creator_t)VPOutPluginAO::VPOutPluginAO_new, VPBUFFER_PERIOD };
    creators.insert(std::pair<std::string, vpout_entry_t> ("AO",def));
    currentOut = "AO";
#endif

#endif
    currentOut=VSettings::getSingleton()->readString("outplugin",currentOut);

}

VPDecoderFactory::VPDecoderFactory()
{
    vpdecoder_entry_t decoders[]= {
     {"FLAC",(vpdecode_creator_t)FLACDecoder::VPDecoderFLAC_new },
     {"MPEG",(vpdecode_creator_t)MPEGDecoder::VPDecoderMPEG_new },
     {"OGG",(vpdecode_creator_t)OGGDecoder::VPDecoderOGG_new }
    };
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("flac",decoders[0]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp1",decoders[1]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp2",decoders[1]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("mp3",decoders[1]));
    creators.insert(std::pair<std::string, vpdecoder_entry_t>("ogg",decoders[2]));
}
