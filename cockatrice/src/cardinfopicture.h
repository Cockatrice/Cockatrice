#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include <QLabel>

class AbstractCardItem;
class CardInfo;

class CardInfoPicture : public QLabel {
    Q_OBJECT

private:
    CardInfo *info;

public:
    CardInfoPicture(QWidget *parent = 0);
private:
    void setNoPicture(bool status);
protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void setCard(CardInfo *card);
    void updatePixmap();
};

#endif
