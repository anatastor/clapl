
#include "playback.h"


void playback_init (void)
{
    av_register_all();
    avcodec_register_all();
    ao_initialize();

    av_log_set_level(AV_LOG_QUIET);
    logcmd(LOG_DMSG, "playback initialized");
}


playback *playback_open_file (const char *path)
{   
    playback *pb = malloc(sizeof(playback));

    pb->first_try = 2;

    pb->ctx = avformat_alloc_context();
    if (avformat_open_input(&pb->ctx, path, NULL, NULL) < 0)
        logcmd(LOG_ERROR, "playback: playback_open_file: coult not open file");

    if (avformat_find_stream_info(pb->ctx, NULL) < 0)
        logcmd(LOG_ERROR, "playback: playback_open_file: could not find stream info");


    AVCodec *codec = avcodec_find_decoder(pb->ctx->streams[0]->codecpar->codec_id);
    if (! codec)
        logcmd(LOG_ERROR, "playback: playback_open_file: could not find codec");

    pb->avctx = avcodec_alloc_context3(codec); if (! pb->avctx)
        logcmd(LOG_ERROR, "playback: playback_open_file: could not allocate decoding context: %s", path);

    if (avcodec_parameters_to_context(pb->avctx, pb->ctx->streams[0]->codecpar) < 0)
        logcmd(LOG_ERROR, "playback: playback_open_file: ERROR: %s", path);

    if (avcodec_open2(pb->avctx, NULL, NULL) < 0)
        logcmd(LOG_ERROR, "playback: playback_open_file: could not open codec: %s", path);
    
    switch (pb->avctx->sample_fmt)
    {
        case AV_SAMPLE_FMT_U8: case AV_SAMPLE_FMT_U8P:
            pb->sformat.bits = 8;
            break;

        case AV_SAMPLE_FMT_S16: case AV_SAMPLE_FMT_S16P:
            pb->sformat.bits = 16;
            break;

        case AV_SAMPLE_FMT_FLT: case AV_SAMPLE_FMT_FLTP:
            pb->sformat.bits = 16;
            break;

        case AV_SAMPLE_FMT_DBL: case AV_SAMPLE_FMT_DBLP:
            pb->sformat.bits = 16;

        default:
            logcmd(LOG_ERROR, "playback: playback_open_file: could not detect format: %s", path);
            break;
        
    }
    
    pb->sformat.channels = pb->avctx->channels;
    pb->sformat.rate = pb->avctx->sample_rate;
    pb->sformat.byte_format = AO_FMT_NATIVE;
    if (pb->sformat.channels > 1)
        pb->sformat.matrix = "L,R";
    else
        pb->sformat.matrix = "C";
    

    int driver = ao_default_driver_id();
    pb->adevice = ao_open_live(driver, &pb->sformat, NULL);

    av_init_packet(&pb->pkt);
    pb->frame = av_frame_alloc();

    return pb;
}


int playback_playback (playback *pb)
{
    int plane_size;

    if (av_read_frame(pb->ctx, &pb->pkt) >= 0)
    {
        if (avcodec_send_packet(pb->avctx, &pb->pkt) < 0 && pb->first_try)
        {
            pb->first_try--;
            logcmd(LOG_DMSG, "error sending packet, trying again");
            return 1;
        }
        else if (! pb->first_try)
        {
            logcmd(LOG_DMSG, "error sending packet");
            return -1;
        }

        if (avcodec_receive_frame(pb->avctx, pb->frame) < 0)
            return 1;

        int data_size = av_samples_get_buffer_size(&plane_size, pb->avctx->channels, pb->frame->nb_samples, pb->avctx->sample_fmt, 1);

        uint16_t *out = (uint16_t*) pb->samples;
        int write = 0;

        switch (pb->avctx->sample_fmt)
        {
            case AV_SAMPLE_FMT_S16:
                ao_play(pb->adevice, (char*) pb->frame->extended_data[0], pb->frame->linesize[0]);
                break;

            case AV_SAMPLE_FMT_S16P:
                for (int nb = 0; nb < plane_size / sizeof(uint16_t); nb++)
                {
                    for (int ch = 0; ch < pb->avctx->channels; ch++)
                    {
                        out[write] = ((uint16_t*) pb->frame->extended_data[ch])[nb];
                        write++;
                    }
                }
                ao_play(pb->adevice, (char*) pb->samples, plane_size * pb->avctx->channels);
                break;

            case AV_SAMPLE_FMT_FLT:
                ao_play(pb->adevice, (char*) pb->frame->extended_data[0], pb->frame->linesize[0]);
                break;

            case AV_SAMPLE_FMT_FLTP:
                for (int nb = 0; nb < pb->frame->nb_samples; nb++)
                {
                    for (int ch = 0; ch < pb->avctx->channels; ch++)
                    {
                        float *extended_data = (float*) pb->frame->extended_data[ch];
                        float sample = extended_data[nb];
                        if (sample < -1.0f)
                            sample = -1.0f;
                        else if (sample > 1.0f)
                            sample = 1.0f;

                        out[nb * pb->avctx->channels + ch] = (uint16_t) round(sample * SHRT_MAX);
                    }
                }
                ao_play(pb->adevice, (char*) pb->samples, (plane_size / sizeof(float)) * sizeof(uint16_t) * pb->avctx->channels);
                break;
        }


    }
    else
        return 0;

    return 1;
}


void playback_seek_timestamp (playback *pb, const int time)
{
    while (av_read_frame(pb->ctx, &pb->pkt) >= 0)
    {
        if (avcodec_send_packet(pb->avctx, &pb->pkt) < 0 && pb->first_try)
        {
            pb->first_try--;
            continue;
        }
        else if (!pb->first_try)
            return;

        if (avcodec_receive_frame(pb->avctx, pb->frame) < 0)
            return;

        int t = av_frame_get_best_effort_timestamp(pb->frame) / pb->ctx->streams[0]->time_base.den;
        if (t >= time)
            return;
    }
}


void playback_free_file (playback *pb)
{
    if (! pb)
        return;
    avformat_close_input(&pb->ctx);
    ao_close(pb->adevice);
}


void playback_shutdown (void)
{
    ao_shutdown();
}

