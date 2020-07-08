#ifndef WORKER_H
#define WORKER_H

#include <QString>
#include <mutex>
#include <thread>
#include <condition_variable>

class CFFmpeg;
class Worker
{
    typedef enum {
        INIT = 0,
        PLAY,
        STOP,
        RELEASE
    } WORK_STATE;

    typedef enum {
        IDLE = 0,
        WORK,
        QUIT
    } STATE;

public:
    explicit Worker();
    ~Worker();

    void play(const QString &url);
    void pause();
    void stop();

private:
    void work();

public:
    CFFmpeg* media() const;

private:
    CFFmpeg *m_ppg;

    std::mutex m_mtx;

    std::thread *m_ptd;
    std::condition_variable m_cv;

    QString m_url;

    STATE m_state;
    WORK_STATE m_wstate;
};

#endif // WORKER_H
