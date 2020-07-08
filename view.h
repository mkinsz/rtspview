#ifndef VIEW_H
#define VIEW_H

#include <QFrame>

namespace Ui {
class View;
}

class Worker;
class View : public QFrame
{
    Q_OBJECT

public:
    explicit View(QWidget *parent = nullptr);
    ~View();

    void clear();
    const QPixmap *pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    bool isMaxed() const;
    void setMaxed(const bool bMax);

protected:
    void dropEvent(QDropEvent *event) override;             // 放下动作
    void dragEnterEvent(QDragEnterEvent *event) override;   // 托到进入窗口动作
    void dragMoveEvent(QDragMoveEvent *event) override;     // 拖着物体在窗口移动
    void dragLeaveEvent(QDragLeaveEvent *event) override;	// 拖走了没有释放

private slots:
    void on_closebtn_clicked();
    void handleFrameChanged(const QImage image);

private:
    void play(const QString &url);
    void stop();

private:
    Ui::View *ui;

    Worker *m_pWorker = nullptr;

    QSize m_size;
    bool m_bMaxed;
    bool m_bFlag = false;   //播放 停止
};

#endif // VIEW_H
