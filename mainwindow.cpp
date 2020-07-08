#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cffmpeg.h"
#include "worker.h"
#include "treeview/treemodel.h"
#include "treeview/itemdelegate.h"
#include <QDebug>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_width=375;
    m_height=211;
    ui->view1->installEventFilter(this);
    ui->view2->installEventFilter(this);
    ui->view3->installEventFilter(this);

    TreeModel *model = new TreeModel(this);
    model->setXML(":/config.xml");

    ui->treeView->setModel(model);
    ui->treeView->expandAll();
    ui->treeView->setAnimated(true);
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
	ui->treeView->setDragDropMode(QAbstractItemView::DragOnly);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(delegate, SIGNAL(expanded(const QModelIndex &)), this, SLOT(handleExpanded(const QModelIndex &)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleCustomContextMenuRequested(QPoint)));
}

MainWindow::~MainWindow()
{
    ui->view1->removeEventFilter(this);
    ui->view2->removeEventFilter(this);
    ui->view3->removeEventFilter(this);

    if(m_pWorker) delete m_pWorker;
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if(obj->metaObject()->className() == QStringLiteral("View")) {
            QList<View *> views = ui->content->findChildren<View *>();
            auto view = static_cast<View*>(obj);
            for(auto m : views)
                if(view != m) m->setVisible(view->isMaxed());
            view->setMaxed(!view->isMaxed());
        }
    }
    return QObject::eventFilter(obj,event);
}

void MainWindow::handleFrameChanged(const QImage image)
{
    if (image.height()>0){
        ui->view1->setPixmap(QPixmap::fromImage(image.scaled(m_width,m_height)));
    }
}

void MainWindow::on_playbtn_clicked()
{
    m_pWorker = new Worker();
    CFFmpeg *ppg = m_pWorker->media();
    connect(ppg, SIGNAL(frameChanged(QImage)), this, SLOT(handleFrameChanged(QImage)), Qt::QueuedConnection);
    m_pWorker->play(ui->lineedit->text());
}

void MainWindow::on_pausebtn_clicked()
{
    if(m_pWorker) m_pWorker->pause();
}

void MainWindow::on_stopbtn_clicked()
{
    if(m_pWorker) {
        m_pWorker->stop();
        delete m_pWorker;
        m_pWorker = nullptr;
    }
}

void MainWindow::on_grabbtn_clicked()
{
    ui->view4->clear();
    int index=ui->chnbox->currentIndex();

    switch(index) {
    case 0: ui->view4->setPixmap(*ui->view1->pixmap()); break;
    case 1: ui->view4->setPixmap(*ui->view2->pixmap()); break;
    case 2: ui->view4->setPixmap(*ui->view3->pixmap()); break;
    }
}

void MainWindow::handleExpanded(const QModelIndex &index)
{
    bool bExpanded = ui->treeView->isExpanded(index);
    if (!bExpanded)
    {
        ui->treeView->expand(index);
    }
    else
    {
        ui->treeView->collapse(index);
    }
}

void MainWindow::handleCustomContextMenuRequested(const QPoint &pos)
{
    if (!m_menu)
    {
        m_menu = new QMenu(this);

        QAction *expandAllAction = new QAction(m_menu);
        QAction *unExpandAllAction = new QAction(m_menu);
        expandAllAction->setText("全部展开");
        unExpandAllAction->setText("全部折叠");
        m_menu->addAction(expandAllAction);
        m_menu->addAction(unExpandAllAction);

        connect(expandAllAction, SIGNAL(triggered()), ui->treeView, SLOT(expandAll()));
        connect(unExpandAllAction, SIGNAL(triggered()), ui->treeView, SLOT(collapseAll()));
    }

    m_menu->exec(this->mapToGlobal(pos));
}
