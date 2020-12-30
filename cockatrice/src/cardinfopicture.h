#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include "carddatabase.h"

#include <QWidget>

class AbstractCardItem;

class CardInfoPicture : public QWidget
{
    Q_OBJECT

private:
    CardInfoPtr info;
    QPixmap resizedPixmap;
    bool pixmapDirty;

public:
    CardInfoPicture(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *);
    void loadPixmap();
public slots:
    void setCard(CardInfoPtr card);
    void updatePixmap();
};

#endif
