#ifndef QFFMPEG_H
#define QFFMPEG_H

//引入ffmpeg头文件
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
}

#include <QObject>
#include <QImage>
#include <mutex>

class CFFmpeg : public QObject
{
    Q_OBJECT

public:
    explicit CFFmpeg(QObject *parent = nullptr);
    ~CFFmpeg() override;

    bool init(const QString &url);
    bool release();

    void play();

    int width() const{ return m_width; }
    int height() const{ return m_height; }

protected:
    void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame);

    void timerEvent(QTimerEvent *) override;

private:
    AVPacket m_pkt;

    AVFrame  *m_pAVFrame = nullptr;
    AVFrame  *m_pAVPicture = nullptr;

    AVCodecContext  *m_pAVCodecCtx = nullptr;
    AVFormatContext *m_pAVFormatCtx = nullptr;

    SwsContext *m_pSwsCtx = nullptr;

    QString m_url;

    int m_width;
    int m_height;
    int m_streamIndex;

    bool m_bInit = false;
    std::mutex m_mtx;

signals:
    void frameChanged(const QImage image);

public slots:

};

#endif // QFFMPEG_H
