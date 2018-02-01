
#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <ao/ao.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include "enums.h"
#include "logger.h"


#define INBUF_SIZE  4096
#define AUDIO_INBUF_SIZE    20480
#define AUDIO_REFILL_THRESH 4096


typedef struct playback
{
    AVFormatContext *ctx;
    AVCodecContext *avctx;

    ao_sample_format sformat;
    ao_device *adevice;

    AVPacket pkt;
    AVFrame *frame;

    uint8_t buffer[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *samples[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];

    char first_try;
} playback;


typedef struct audio
{
    playback *pb;
    char playstate;
    char threadstate;
    char cycle;
} audio;


void playback_init (void);

audio *audio_create (void); // creates and initialzes audio

playback *playback_open_file (const char *path);
int playback_playback (playback *pb);
/*
 * return
 * 1 if file ist still playing
 * -1 on error
 * 0 at EOF
 */

void playback_seek_timestamp (playback *pb, const int time);

void playback_free_file (playback *pb);


void playback_shutdown (void);


#endif
