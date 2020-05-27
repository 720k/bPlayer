#include "DropLabel.h"
#include <QMimeData>
#include <QDropEvent>
#include <QDebug>
DropLabel::DropLabel(QWidget *parent) : QLabel(parent) {
    setAcceptDrops(true);
}

void DropLabel::dragEnterEvent(QDragEnterEvent *event)  {
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().count()==1)       event->acceptProposedAction();
}

void DropLabel::dropEvent(QDropEvent *event)    {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls())     {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();
        for (const auto& url : urlList) {
//            qDebug() << url.toLocalFile();
            emit droppedFile(url.toLocalFile());
        }
    }
}
