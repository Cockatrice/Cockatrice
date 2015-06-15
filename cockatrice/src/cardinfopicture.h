#ifndef CARDINFOPICTURE_H
#define CARDINFOPICTURE_H

#include <QLabel>

class AbstractCardItem;
class CardInfo;

class CardInfoPicture : public QLabel {
    Q_OBJECT

signals:
    void hasPictureChanged();

private:
    CardInfo *info;
    bool noPicture;

public:
    CardInfoPicture(int maximumWidth, QWidget *parent = 0);
    bool hasPicture() const { return !noPicture; }
private:
    void setNoPicture(bool status);
protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void setCard(CardInfo *card);
    void updatePixmap();
};

#endif
