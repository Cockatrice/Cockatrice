#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include <QWidget>

class AbstractCardItem;
class CardInfo;

class CardInfoPicture : public QWidget {
    Q_OBJECT

private:
    CardInfo *info;
    QPixmap resizedPixmap;
    bool pixmapDirty;

public:
    CardInfoPicture(QWidget *parent = 0);
protected:
    void resizeEvent(QResizeEvent *event);
	void paintEvent(QPaintEvent *);
	void loadPixmap();
public slots:
    void setCard(CardInfo *card);
    void updatePixmap();
};

#endif
