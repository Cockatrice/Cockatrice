#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include "../../../../game/cards/card_database.h"

#include <QWidget>

class AbstractCardItem;

class CardInfoPictureWidget : public QWidget
{
    Q_OBJECT

private:
    CardInfoPtr info;
    QPixmap resizedPixmap;
    bool pixmapDirty;

public:
    CardInfoPictureWidget(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *);
    void loadPixmap();
public slots:
    void setCard(CardInfoPtr card);
    void updatePixmap();
};

#endif
