#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QScreen>

//#include <iostream>
//#include <atomic>
//#include <thread>
//#include <vector>

//std::atomic<bool> ready(false);
//std::atomic_flag winner = ATOMIC_FLAG_INIT;

//void count1m(int id) {
//    while(!ready) { std::this_thread::yield(); }
//    for(volatile int i = 0; i < 1000000; ++i) {}
//    if(!winner.test_and_set()) {
//        std::cout << "thread #" << id << " won!\n";
//    }
//}

//int main() {
//    std::vector<std::thread> threads;
//    std::cout << "spawning 10 threads that count to 1 million...\n";
//    for(int i = 1; i <= 10; ++i)
//        threads.push_back(std::thread(count1m, i));
//    ready = true;
//    for(auto &th : threads) th.join();

//    return 0;
//}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen()){
        qApp->setStyleSheet(QLatin1String(qssFile.readAll()));
        qssFile.close();
    }

    MainWindow w;
    int frmX = w.width();
    int frmY = w.height();

    QScreen *ps = qApp->primaryScreen();
    int screenX = ps->availableGeometry().width();
    int screenY = ps->availableGeometry().height();
    w.move(screenX/2-frmX/2, screenY/2-frmY/2);

    w.setFixedSize(frmX,frmY);
    w.show();

    return a.exec();
}
