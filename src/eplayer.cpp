#include "eplayer.h"

#include "elog/elog.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

VideoInfo::VideoInfo(/* args */) {}

VideoInfo::~VideoInfo() {}

int VideoInfo::readFile()
{

    AVPacket *packet = av_packet_alloc();
    int ret = av_read_frame(input_c, packet);
    if (ret == 0)
    {
        // video stream data
        LOG_D(video_s_index, " ", packet->stream_index)
        if (video_s_index == packet->stream_index)
        {

            std::unique_lock<std::mutex> lk(avpacket_queue_mutex);

            avpacket_queue_condition.wait(
                lk,
                [this]
                {
                    LOG_D(avpacket_queue.size())
                    return this->avpacket_queue.size() < 20;
                });
            avpacket_queue.emplace(packet);
            goto out;
        }
    }

    av_packet_free(&packet);
out:
    return ret;
}

double VideoInfo::getVideoFrameTimeBase()
{

    LOG_I(video_stream->time_base.num, " ", video_stream->time_base.den)
    return (video_stream->time_base.num / (double)video_stream->time_base.den);
}

int VideoInfo::getVideoFrame(AVFrame *frame)
{
    int ret;
    {
        std::unique_lock<std::mutex> lk(avpacket_queue_mutex);

        if (!avpacket_queue.empty())
        {
            if (avcodec_send_packet(v_avctx, avpacket_queue.front()) == 0)
            {
                av_packet_free(&(avpacket_queue.front()));
                avpacket_queue.pop();

                LOG_D(avpacket_queue.size())
                avpacket_queue_condition.notify_one();
            }
            else
            {
                LOG_E("codec ", avcodec_get_name(v_avctx->codec_id))

                LOG_E(avcodec_is_open(v_avctx), "  ",
                      avcodec_send_packet(v_avctx, avpacket_queue.front()))

                throw std::runtime_error("avcodec_send_packet err");
            }
        }
    }
    ret = avcodec_receive_frame(v_avctx, frame);
    if (ret != 0 && ret != AVERROR(EAGAIN))
    {
        LOG_E("avcodec_receive_frame  ", ret)
    }

    return ret;
}

int VideoInfo::scaleFrameToImage(AVFrame *frame, AVPixelFormat format,
                                 ImageDate &image)
{
    // ImageDate image;

    if (!frame)
    {
        throw std::runtime_error("frame nullptr\n");
    }

    image.height = frame->height;
    image.width = frame->width;
    image.pts = frame->pts;

    LOG_D(frame->pts)

    int dst_linesize[4];

    if (av_image_alloc(image.data, dst_linesize, frame->width, frame->height,
                       static_cast<AVPixelFormat>(format), 1) < 0)
    {
        throw std::runtime_error("av_image_alloc failed\n");
    }
    sws_ctx = sws_getCachedContext(
        sws_ctx, frame->width, frame->height,
        static_cast<AVPixelFormat>(frame->format), frame->width, frame->height,
        static_cast<AVPixelFormat>(format), SWS_BICUBIC, NULL, NULL, NULL);
    if (sws_ctx != NULL)
    {
        sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                  0, frame->height, image.data, dst_linesize);
    }
    else
    {
        throw std::runtime_error("Cannot initialize the conversion context\n");
    }

    return 0;
}

int VideoInfo::getStreamCodec(uint32_t stream_index)
{
    AVCodecContext *avctx;
    const AVCodec *codec;
    int ret;

    avctx = avcodec_alloc_context3(NULL);

    //获取解码器
    ret = avcodec_parameters_to_context(
        avctx, input_c->streams[stream_index]->codecpar);
    if (ret < 0)
        goto fail;
    avctx->pkt_timebase = input_c->streams[stream_index]->time_base;

    codec = avcodec_find_decoder(avctx->codec_id);
    if (!codec)
    {
        goto fail;
    }
    avctx->codec_id = codec->id;

    if ((ret = avcodec_open2(avctx, codec, NULL)) < 0)
    {
        goto fail;
    }

    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:

        v_avctx = avctx;
        video_s_index = stream_index;
        video_stream = input_c->streams[stream_index];

        break;
    }

    goto out;

fail:
    avcodec_free_context(&avctx);
out:
    return ret;
}

int VideoInfo::openFile(const std::string &file_url)
{

    AVFormatContext *ic = NULL;

    ic = avformat_alloc_context();
    if (!ic)
    {
        goto fail;
    }

    LOG_I(file_url)

    if (avformat_open_input(&ic, file_url.c_str(), nullptr, nullptr))
    {
        goto fail;
    }
    if (avformat_find_stream_info(ic, nullptr))
    {
        goto fail;
    }

    int strean_index[AVMEDIA_TYPE_NB];

    strean_index[AVMEDIA_TYPE_VIDEO] = -1;
    strean_index[AVMEDIA_TYPE_AUDIO] = -1;
    strean_index[AVMEDIA_TYPE_SUBTITLE] = -1;

    for (uint32_t i = 0; i < ic->nb_streams; i++)
    {
        if (strean_index[AVMEDIA_TYPE_VIDEO] == -1 &&
            ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            strean_index[AVMEDIA_TYPE_VIDEO] = i;
            printf("Find a video stream, index %d\n",
                   strean_index[AVMEDIA_TYPE_VIDEO]);
            // break;
        }
        if (strean_index[AVMEDIA_TYPE_AUDIO] == -1 &&
            ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            strean_index[AVMEDIA_TYPE_AUDIO] = i;
            printf("Find a audio stream, index %d\n",
                   strean_index[AVMEDIA_TYPE_AUDIO]);
            // break;
        }
        if (strean_index[AVMEDIA_TYPE_SUBTITLE] == -1 &&
            ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
        {
            strean_index[AVMEDIA_TYPE_SUBTITLE] = i;
            printf("Find a subtitle stream, index %d\n",
                   strean_index[AVMEDIA_TYPE_SUBTITLE]);
            // break;
        }
        if (strean_index[AVMEDIA_TYPE_VIDEO] &&
            strean_index[AVMEDIA_TYPE_AUDIO] &&
            strean_index[AVMEDIA_TYPE_SUBTITLE])
        {
            break;
        }
    }

    input_c = ic;

    if (strean_index[AVMEDIA_TYPE_VIDEO] != -1 &&
        getStreamCodec(strean_index[AVMEDIA_TYPE_VIDEO]) < 0)
    {
        goto fail;
    }

    return 0;

fail:
    avformat_free_context(ic);
    return -1;
}

EPlayer::EPlayer(/* args */) {}

EPlayer::~EPlayer() {}

void EPlayer::setUrl(std::string url)
{
    file_url = url;
    return;
}

int64_t EPlayer::checkCurrentFrameTime(uint64_t frame_pts)
{
    if (frame_pts == 0)
    {
        return 0;
    }

    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now() - last_frame_tp)
                    .count() +
                last_frame_df;
    // if (time > (time_base * 1000))
    // {
    //     return 1;
    // }
    return time - (frame_pts - last_frame_pts) * time_base * 1000;
}

uint8_t EPlayer::run()
{

    // LOG_SET_lEVEL(LogLevel::LOG_DEBUG);

    // view.drawLoop();

    draw_tid = std::thread(
        [this]
        {
            this->view.createWindow(std::string("EPlayer"));
            this->view.initVulkan();
            this->view.drawLoop();
            return;
        });
    LOG_D(draw_tid.get_id())

    if (vinfo.openFile(file_url.c_str()) == 0)
    {
        using namespace std;

        if ((time_base = static_cast<uint64_t>(
                 (vinfo.getVideoFrameTimeBase() * 1000))) < 0)
        {
            throw runtime_error("time_base err ");
        }

        read_tid = thread(
            [this]
            {
                while (1)
                {
                    this->vinfo.readFile();
                    // this_thread::sleep_for(chrono::microseconds(1));
                }
            });
        LOG_D(read_tid.get_id())
#if 1
        view_tid = thread(
            [this]
            {
                thread cktime_tid;
                int cktime_isrun = -1;

                ThreadPool cktime(1);
                while (true)
                {
                    {

                        unique_lock<mutex> lk(this->image_queue_mutex);
                        this->image_queue_condition_r.wait(
                            lk, [this, &cktime_tid, &cktime_isrun, &cktime]
                            { return !this->image_queue.empty(); });
                        auto &image = this->image_queue.front();
                        int64_t ct = this->checkCurrentFrameTime(
                            this->image_queue.front().pts);
                        while (ct < 0 || this->view.isRun() != 1)
                        {

                            // if (ct < -1500 || this->view.isRun() != 1)
                            // {
                            this->image_queue_condition_r.wait_for(
                                lk, chrono::microseconds(1));
                            // }
                            ct = this->checkCurrentFrameTime(
                                this->image_queue.front().pts);
                            // LOG_I(ct)
                        }

                        // if (ct > 0)
                        // {
                        //     LOG_I(ct)
                        // }

                        this->last_frame_df = ct;
                        this->last_frame_tp = chrono::system_clock::now();
                        this->last_frame_pts = image.pts;

                        view.updateTextureImage(image.data[0], image.width,
                                                image.height);

                        av_freep(&image.data[0]);

                        this->image_queue.pop();
                        this->image_queue_condition.notify_one();
                    }
                    this_thread::sleep_for(chrono::milliseconds(1));
                }
            });
#endif
        AVFrame *frame = av_frame_alloc();
        while (true)
        {

            if (vinfo.getVideoFrame(frame) == 0)
            {
                ImageDate image;
                if (vinfo.scaleFrameToImage(frame, AV_PIX_FMT_RGBA, image) == 0)
                {
#if 1

                    unique_lock<mutex> lk(image_queue_mutex);
                    image_queue_condition.wait(
                        lk, [this] { return this->image_queue.size() < 10; });
                    image_queue.emplace(image);
                    image_queue_condition_r.notify_one();
                    continue;
#else
                    view.updateTextureImage(image.data[0], image.width,
                                            image.height);

#endif
                }
                av_freep(&image.data[0]);
            }
            // this_thread::sleep_for(chrono::milliseconds(1));
        }
    }

    return 0;
}