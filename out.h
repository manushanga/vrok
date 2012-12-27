/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef OUT_H
#define OUT_H

#include <stdint.h>
#include <map>

#include "vputils.h"
#include "vplayer.h"

class VPOutPlugin{
public:
    bool work;

    virtual int init(VPlayer *v, unsigned samplerate, unsigned channels) = 0;
    virtual void resume() = 0;
    virtual void pause() = 0;
    virtual unsigned get_samplerate() = 0;
    virtual unsigned get_channels() = 0;
    virtual ~VPOutPlugin() {}
};

#endif // OUT_H
