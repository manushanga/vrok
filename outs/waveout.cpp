#include "waveout.h"

VPOutPlugin* _VPOutPluginWaveOut_new()
{
    return (VPOutPlugin *) new VPOutPluginWaveOut();
}


int VPOutPluginWaveOut::init(VPlayer *v, unsigned samplerate, unsigned channels)
{

}
unsigned VPOutPluginWaveOut::get_channels()
{
    return 2;
}
unsigned VPOutPluginWaveOut::get_samplerate()
{
    return 44100;
}

void VPOutPluginWaveOut::pause()
{
}
void VPOutPluginWaveOut::resume()
{
}

int VPOutPluginWaveOut::finit()
{
}


