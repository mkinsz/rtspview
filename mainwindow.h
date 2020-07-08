#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QMenu;
class Worker;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleFrameChanged(const QImage image);

    void on_playbtn_clicked();
    void on_pausebtn_clicked();
    void on_stopbtn_clicked();
    void on_grabbtn_clicked();

private slots:
    void handleExpanded(const QModelIndex &index);
    void handleCustomContextMenuRequested(const QPoint &pos);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    int m_width;
    int m_height;

private:
    Ui::MainWindow *ui;
    Worker *m_pWorker = nullptr;
    QMenu *m_menu;
};
#endif // MAINWINDOW_H
