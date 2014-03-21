/*
  Vrok - smokin' audio
  (C) 2013 Madura A. released under GPL 2.0 All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef FFMPEGPLAYER_H
#define FFMPEGPLAYER_H

#include "vplayer.h"

extern "C" {

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#define FFMPEG_MAX_BUF_SIZE 192000

class FFMPEGDecoder : public VPDecoderPlugin
{
private:
    AVFormatContext* container;
    int audio_stream_id;
    AVCodecContext *ctx;
    AVCodec *codec;
    AVSampleFormat sfmt;
    AVFrame *frame;
    AVPacket packet;
    ALIGNAUTO (float remainder[FFMPEG_MAX_BUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]);
    uint64_t frame_position;
    uint64_t seek_to;
    uint64_t frame_count;
public:
    FFMPEGDecoder(VPlayer *v);
    static VPDecoderPlugin* VPDecoderFFMPEG_new(VPlayer *v);
    int open(const char *url);
    void reader();
    uint64_t getLength();
    void setPosition(uint64_t t);
    uint64_t getPosition();
    ~FFMPEGDecoder();
};

#endif // FFMPEGPLAYER_H
