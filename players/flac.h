/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef PLAYER_FLAC_H
#define PLAYER_FLAC_H

#include <FLAC/stream_decoder.h>

#include "vplayer.h"
#include "decoder.h"

class FLACDecoder : public VPDecoder
{
private:
    FILE *fcurrent;
    float to_fl;
    static void metadata_callback(const FLAC__StreamDecoder *decoder,
                             const FLAC__StreamMetadata *metadata,
                             void *client_data);
    static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder,
                                                         const FLAC__Frame *frame,
                                                         const FLAC__int32 * const buffer[],
                                                         void *client_data);
public:
    static VPDecoder* VPDecoderFLAC_new();
    FLAC__StreamDecoder *decoder;
    FLAC__StreamDecoderInitStatus init_status;
    float *buffer;
    unsigned buffer_write;
    unsigned half_buffer_bytes;
    int ret_vpout_open;
    VPlayer *owner;

    FLACDecoder();
    void init(VPlayer *v);
    int open(const char *url);
    void reader();
    unsigned long getLength();
    void setPosition(unsigned long t);
    unsigned long getPosition();
    ~FLACDecoder();
};

#endif // PLAYER_FLAC_H
