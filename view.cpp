#include "view.h"
#include "ui_view.h"

#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QVariant>
#include <QDebug>

#include "worker.h"
#include "cffmpeg.h"

View::View(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::View)
{
    ui->setupUi(this);

    setMaxed(false);
    setAcceptDrops(true);   //允许接受拖拽事件

    ui->header->setEnabled(false);
    ui->header->setVisible(false);
}

View::~View()
{
    delete ui;
}

void View::clear()
{
    ui->label->clear();
}

const QPixmap *View::pixmap() const
{
    return ui->label->pixmap();
}

void View::setPixmap(const QPixmap &pixmap)
{
    ui->label->setPixmap(pixmap);
}

bool View::isMaxed() const
{
    return m_bMaxed;
}

void View::setMaxed(const bool bMax)
{
    m_bMaxed = bMax;
    m_size.setWidth(bMax ? 720 : 375);
    m_size.setHeight(bMax ? 405 : 211);
    resize(bMax ? 720 : 375, this->height());
}

void View::dropEvent(QDropEvent *event)
{
    QStringList formats = event->mimeData()->formats();
    qDebug() << "drop formats = " << formats;
    if (event->mimeData()->hasFormat("address"))
    {
        QByteArray url = event->mimeData()->data("address");
        if(!url.isEmpty()) {
            play(url);
            ui->header->setEnabled(true);
            ui->header->setVisible(true);
        }

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void View::dragEnterEvent(QDragEnterEvent *event)
{
    QStringList formats = event->mimeData()->formats();
    qDebug() << "drag enter formats = " << formats;
    if (event->mimeData()->hasFormat("name"))
        event->accept();
    else
        event->ignore();
}

void View::dragMoveEvent(QDragMoveEvent *event)
{
    QStringList formats = event->mimeData()->formats();
    qDebug() << "drag move formats = " << formats;
    if (event->mimeData()->hasFormat("name")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void View::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "drag leave...";
}

void View::on_closebtn_clicked()
{
    stop();
    clear();

    ui->header->setEnabled(false);
    ui->header->setVisible(false);
}

void View::handleFrameChanged(const QImage image)
{
    if(!m_bFlag) return;

    qDebug() << "Frame Change...";

    if (image.height() > 0) {
        QImage nimage = image.scaled(m_size);
        ui->label->setPixmap(QPixmap::fromImage(nimage));
    }
}

void View::play(const QString &url)
{
    if(!m_pWorker) {
        m_pWorker = new Worker();
        CFFmpeg *ppg = m_pWorker->media();
        connect(ppg, SIGNAL(frameChanged(const QImage)), this, SLOT(handleFrameChanged(const QImage)), Qt::QueuedConnection);
    }
    m_pWorker->play(url);

    m_bFlag = true;
}

void View::stop()
{
    m_bFlag = false;

    if(m_pWorker) {
        m_pWorker->stop();
        delete m_pWorker;
        m_pWorker = nullptr;
    }
}
