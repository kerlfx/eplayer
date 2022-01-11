#ifndef EPLAYER_H_
#define EPLAYER_H_
#include <chrono>
#include <queue>
#include <stdexcept>
#include <string>

#include "vulkan/vulkanview.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

typedef struct ImageDate
{
    uint8_t *data[4];
    int width;
    int height;
    int64_t pts;
} ImageDate;

// typedef void *VulkanViewP;
class VideoInfo
{
private:
    AVFormatContext *input_c;

    struct SwsContext *sws_ctx = nullptr;

    int video_s_index = -1;
    AVStream *video_stream = nullptr;
    AVCodecContext *v_avctx = nullptr;

    std::thread read_tid;

    std::thread v_tid;

    int16_t avpacket_queue_max_s;
    std::queue<AVPacket *> avpacket_queue;
    std::mutex avpacket_queue_mutex;
    std::condition_variable avpacket_queue_condition;

    int getStreamCodec(uint32_t stream_index);

public:
    VideoInfo(/* args */);
    ~VideoInfo();

    double getVideoFrameTimeBase();

    int getVideoFrame(AVFrame *frame);

    int scaleFrameToImage(AVFrame *frame, AVPixelFormat format,
                          ImageDate &image);

    // int getVideoCodec() { getStreamCodec(video_s_index); }

    /**
     * 返回 0 成功 非0 失败
     * */
    int openFile(const std::string &file_url);
    int readFile();
};

class EPlayer
{
private:
    std::string file_url;

    VideoInfo vinfo;

    VulkanView view;
    std::thread draw_tid;
    std::thread read_tid;

    std::queue<ImageDate> image_queue;
    std::mutex image_queue_mutex;
    std::condition_variable image_queue_condition;
    std::condition_variable image_queue_condition_r;

    uint64_t last_frame_pts = 0; // 正在显示的帧在视频中的时间
    uint64_t time_base = 0;      //单位 毫秒

    std::chrono::system_clock::time_point
        last_frame_tp;     // 正在显示的帧在实际中的时间
    int64_t last_frame_df; // 正在显示的帧在实际中相对偏移

    std::thread view_tid;

    int64_t checkCurrentFrameTime(uint64_t frame_pts);

public:
    void setUrl(std::string url);
    const std::string &getFileUrl() { return file_url; }

    enum
    {
        NOURL = 1,
    };

    /**
     * 返回：0 成功 非0 失败
     * */
    uint8_t run();

    EPlayer(/* args */);
    ~EPlayer();
};

#endif // EPLAYER_H_
