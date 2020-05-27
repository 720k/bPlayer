#pragma once

#include <QLabel>

class DropLabel : public QLabel
{
    Q_OBJECT
public:
                    DropLabel(QWidget *parent = nullptr);

        void        dragEnterEvent(QDragEnterEvent *event) override;
        void        dropEvent(QDropEvent* event) override;
signals:
        void        droppedFile(const QString& filename);
};

