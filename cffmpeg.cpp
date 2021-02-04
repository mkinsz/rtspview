#include "cffmpeg.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>

//ffmpeg -re -rtsp_transport tcp -i rtsp://usr:passwd@ip:port/h264/ch1/sub/av_stream -acodec copy -vcodec copy -f rtsp rtsp://ip:port/name
//ffmpeg -re -i E:/music/MV/IU.mp4 -vcodec libx264 -vcodec copy -f rtsp rtsp://localhost:8554/live

CFFmpeg::CFFmpeg(QObject *parent) :
    QObject(parent)
{    
    m_streamIndex=-1;
    avdevice_register_all();    // 注册库中所有可用的文件格式和解码器
    avformat_network_init();    // 初始化网络流格式,使用RTSP网络流时必须先执行
    qDebug() << "CFFmpeg: " << QThread::currentThreadId();
}

CFFmpeg::~CFFmpeg()
{
    qDebug() << "~CFFmpeg: " << QThread::currentThreadId();
    release();
    avformat_network_deinit();
}

bool CFFmpeg::init(const QString &url)
{
    if(m_bInit) return true;
    m_url = url;

    m_pAVFrame = av_frame_alloc();
    m_pAVPicture = av_frame_alloc();
    m_pAVFormatCtx = avformat_alloc_context();  // 申请AVFormatContext结构的内存

    AVDictionary* options = nullptr;
    //    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "3000000", 0);
    int result = avformat_open_input(&m_pAVFormatCtx, m_url.toStdString().c_str(), nullptr, &options);    // 打开视频流
    if (result < 0) {
        qDebug() << QStringLiteral("打开视频流失败...") << result << AVERROR(EINVAL);
        return false;
    }

    result = avformat_find_stream_info(m_pAVFormatCtx, nullptr);  // 获取视频流信息
    if (result < 0) {
        qDebug() << QStringLiteral("获取视频信息失败...");
        return false;
    }

    m_streamIndex = -1;
    for (uint i = 0; i < m_pAVFormatCtx->nb_streams; i++) {     // 获取视频流索引
        qDebug()<< "Type:" << m_pAVFormatCtx->streams[i]->codecpar->codec_type;
        if (m_pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_streamIndex = static_cast<int>(i);
            break;
        }
    }

    if (m_streamIndex == -1){
        qDebug() << QStringLiteral("获取视频流索引失败...");
        return false;
    }

    AVStream *stream = m_pAVFormatCtx->streams[m_streamIndex];  // 获取视频流的分辨率大小
    AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
    if(!dec) {
        qDebug() << QStringLiteral("找不到该流的解码器...");
        return false;
    }

    m_pAVCodecCtx = avcodec_alloc_context3(dec);    // 为流分配解码器上下文
    if(!m_pAVCodecCtx) {
        qDebug() << QStringLiteral("无法为流分配解码器上下文...");
        return false;
    }

    result = avcodec_parameters_to_context(m_pAVCodecCtx, stream->codecpar);    //
    if(result < 0) {
        qDebug() << QStringLiteral("无法将解码器参数复制到流的输入解码器上下文...");
        return false;
    }

    m_width=m_pAVCodecCtx->width;
    m_height=m_pAVCodecCtx->height;

    m_pAVPicture->width = m_width;
    m_pAVPicture->height = m_height;
    m_pAVPicture->format = AV_PIX_FMT_RGB24;
    if((result = av_frame_get_buffer(m_pAVPicture, 1)) < 0) {
        qDebug() << QStringLiteral("无法分配帧数据...");
        return false;
    }

    AVCodec *pAVCodec = avcodec_find_decoder(m_pAVCodecCtx->codec_id);  // 获取视频流解码器
    m_pSwsCtx = sws_getContext(m_width, m_height, AV_PIX_FMT_YUV420P, m_width, m_height,
                               AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
    result=avcodec_open2(m_pAVCodecCtx, pAVCodec, nullptr);             // 打开对应解码器
    if (result<0) {
        qDebug() << QStringLiteral("打开解码器失败...");
        return false;
    }

    m_bInit = true;
    qDebug() << QStringLiteral("初始化视频流成功...");
    return true;
}

bool CFFmpeg::release()
{
    if(!m_bInit) return true;
    qDebug() << "CFFmpeg release...";

    m_bInit = false;

    if(m_pAVCodecCtx) {
        avcodec_free_context(&m_pAVCodecCtx);
        m_pAVCodecCtx = nullptr;
    }

    if(m_pAVFormatCtx) {
        avformat_close_input(&m_pAVFormatCtx);
        avformat_free_context(m_pAVFormatCtx);
        m_pAVFormatCtx = nullptr;
    }

    if(m_pAVFrame) {
        av_frame_free(&m_pAVFrame);
        m_pAVFrame = nullptr;
    }

    if(m_pAVPicture) {
        av_frame_free(&m_pAVPicture);
        m_pAVPicture = nullptr;
    }


    if(m_pSwsCtx) {
        sws_freeContext(m_pSwsCtx);
        m_pSwsCtx = nullptr;
    }

    return true;
}

void CFFmpeg::play()
{
    if(!m_bInit) return;
    if (av_read_frame(m_pAVFormatCtx, &m_pkt) >= 0){    // 读取视频
        if(m_pkt.stream_index==m_streamIndex){
            qDebug()<< "Start Decode..." << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

            if(m_pkt.size) decode(m_pAVCodecCtx, &m_pkt, m_pAVFrame);
        }
    }

    av_packet_unref(&m_pkt);    // 释放资源`
}

void CFFmpeg::decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame)
{
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if(ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        fflush(stdout);

//        if(frame->key_frame) {

//        }

        sws_scale(m_pSwsCtx, static_cast<const uint8_t* const *>(frame->data), frame->linesize, 0, frame->height, m_pAVPicture->data, m_pAVPicture->linesize);
        qDebug() << "Play: " << QThread::currentThreadId()
                 << m_width << m_height
                 << frame->width << frame->height << dec_ctx->frame_number;

 		QImage image(m_pAVPicture->data[0], m_width, m_height, QImage::Format_RGB888);
        emit frameChanged(image);
    }
}

void CFFmpeg::timerEvent(QTimerEvent *)
{
    //    AVStream *stream=pAVFormatContext->streams[m_pkt.stream_index];
    //    int frame_rate=stream->avg_frame_rate.num/stream->avg_frame_rate.den;//每秒多少帧
    //    qDebug() << "帧数:" << frame_rate;
}
