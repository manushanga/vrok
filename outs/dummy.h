/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef OUT_DUMMY_H
#define OUT_DUMMY_H

#include <samplerate.h>
#include <cstdlib>

#include "vplayer.h"
#include "out.h"


class VPOutPluginDummy: public VPOutPlugin {
public:
    static VPOutPlugin* VPOutPluginDummy_new();
    static void worker_run(VPOutPluginDummy *self);
    VPlayer *owner;
    std::thread *worker;
    std::shared_mutex m_pause;
    volatile bool work;
    volatile bool pause_check;
    volatile bool paused;
    unsigned in_srate, out_srate;
    SRC_STATE *rs;
    SRC_DATA rd;
    float *out_buf;
    unsigned out_frames;

    virtual int init(VPlayer *v, VPBuffer *in);
    virtual void rewind();
    virtual void resume();
    virtual void pause();
    virtual ~VPOutPluginDummy();
};


#endif // OUT_DUMMY_H

