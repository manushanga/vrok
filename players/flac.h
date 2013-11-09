/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef PLAYER_FLAC_H
#define PLAYER_FLAC_H

#define FLAC__NO_DLL

#include <FLAC/stream_decoder.h>

#include "vplayer.h"
#include "decoder.h"

class FLACDecoder : public VPDecoderPlugin
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
    static VPDecoderPlugin* VPDecoderFLAC_new(VPlayer *v);
    FLAC__StreamDecoder *decoder;
    FLAC__StreamDecoderInitStatus init_status;
    float *buffer;
    unsigned buffer_write;
    unsigned buffer_bytes;
    int ret_vpout_open;
    uint64_t seek_to;
    int bps;
    FLACDecoder(VPlayer *v);
    int open(const char *url);
    void reader();
    uint64_t getLength();
    void setPosition(uint64_t t);
    uint64_t getPosition();
    ~FLACDecoder();
};

#endif // PLAYER_FLAC_H
