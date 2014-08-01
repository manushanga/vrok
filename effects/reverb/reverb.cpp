/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <cstring>
#include "vputils.h"
#include "reverb.h"

#define CONV(x,y,z) (x+y*z)/(1.0f+z)
#define WRAPINC(x,max) (x+1) % max
#define CLIP(x) max(min(x,1.0f),-1.0f)

VPEffectPluginReverb::VPEffectPluginReverb() : reverb_buffer(NULL), initd(false)
{
	for (int i=0;i<MAX_REVERBS;i++) { reverb_delay[i]=0; reverb_amp[i]=0.8f; }
}

VPEffectPluginReverb::~VPEffectPluginReverb()
{
}

int VPEffectPluginReverb::init(VPlayer *v, VPBuffer *in, VPBuffer **out)
{

    owner = v;
    bin = in;
    bout = in;
    *out = in;

    reverb_buffer = (float*) ALIGNED_ALLOC(VPBUFFER_FRAMES*bin->chans*sizeof(float)*2);
	
	for (int i=0;i<VPBUFFER_FRAMES*bin->chans*2;i++) { reverb_buffer[i] = 0.0f; }
	
	reverb_read = 0;
    reverb_write = 0;
	buffer_counter = 0;

	initd=true;
    return 0;
}

void VPEffectPluginReverb::process()
{

    float *buffer = bin->currentBuffer();
    int samples = VPBUFFER_FRAMES*bin->chans;

    register int maxx=samples*2;
	

    for (register int i=0;i<samples;i++) {
        reverb_buffer[reverb_write]=buffer[i];
        reverb_write=WRAPINC(reverb_write,maxx);
    }
	if (buffer_counter < 2) {
		buffer_counter++;
	}
    if (buffer_counter < 2) {
        for (register int i=0;i<samples;i++) {
            buffer[i]=0.0f;
        }
    } else {
		float tmp;
        for (register int i=0;i<samples;i++) {
			tmp=CONV(reverb_buffer[reverb_read],reverb_buffer[(reverb_read+reverb_delay[0]) % maxx], reverb_amp[0]);
			for (register int rev=1;(reverb_delay[rev] > 0) && (rev < MAX_REVERBS);rev++) {
				tmp=CONV(tmp, reverb_buffer[(reverb_read+reverb_delay[rev]) % maxx], reverb_amp[rev]);
			}
			buffer[i]=tmp;
			reverb_read=WRAPINC(reverb_read,maxx);
		}       
    }
}

int VPEffectPluginReverb::finit()
{
	if (reverb_buffer)
	    ALIGNED_FREE(reverb_buffer);
    reverb_buffer = NULL;

    return 0;
}
