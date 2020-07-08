#include "worker.h"
#include "cffmpeg.h"

#include <thread>
#include <iostream>
#include <sstream>
#include <functional>
#include <QDebug>

Worker::Worker()
{
    m_state = IDLE;
    m_wstate = INIT;
    m_ppg = new CFFmpeg();
    m_ptd = new std::thread(&Worker::work, this);
}

Worker::~Worker()
{
    qDebug() << "be going to release worker...";
    {
        std::lock_guard<std::mutex> lck(m_mtx);
        m_state = QUIT;
        m_cv.notify_one();
    }

    if(m_ptd->joinable()) m_ptd->join();

    delete m_ptd;
    delete m_ppg;
}

void Worker::play(const QString &url)
{
    std::lock_guard<std::mutex> guard(m_mtx);
    m_url = url;
    m_state = WORK;
    m_cv.notify_one();
}

void Worker::pause()
{
    std::lock_guard<std::mutex> guard(m_mtx);
    if(m_state != IDLE) {
        m_state = IDLE;
        m_cv.notify_one();
    }
}

void Worker::stop()
{
    //    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    qDebug() << "stop worker...";
    std::lock_guard<std::mutex> guard(m_mtx);
    m_state = IDLE;
    m_cv.notify_one();
}

void Worker::work()
{
    while(true) {
        // Acquire the lock
        std::unique_lock<std::mutex> lck(m_mtx);
        m_cv.wait(lck, [&]{ return m_state != IDLE; });

        STATE state = m_state;
        WORK_STATE wstate = m_wstate;
        lck.unlock();

        switch(state) {
        case IDLE: qDebug() << "IDLE..."; break;
        case WORK: {
            switch(wstate) {
            case INIT: {
                if(m_ppg->init(m_url)) {
                    lck.lock();
                    m_wstate = PLAY;
                    lck.unlock();
                }else {
                    lck.lock();
                    m_state = IDLE;
                    lck.unlock();
                }
            }break;
            case PLAY: m_ppg->play(); break;
            default:;
            }
        } break;
        case QUIT: goto QUIT;
        }
    }

QUIT:
    m_ppg->release();
//    std::thread::id id = std::this_thread::get_id();
//    std::stringstream sin;
//    sin << id;
//    qDebug() << "Work Done..." << QString::fromStdString(sin.str());
}

CFFmpeg *Worker::media() const
{
    return m_ppg;
}



