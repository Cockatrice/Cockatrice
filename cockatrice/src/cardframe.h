#ifndef CARDFRAME_H
#define CARDFRAME_H

#include <QStackedWidget>

class AbstractCardItem;
class CardInfo;
class CardInfoPicture;
class CardInfoText;

class CardFrame : public QStackedWidget {
    Q_OBJECT

private:
    CardInfo *info;
    CardInfoPicture *pic;
    CardInfoText *text;
    bool cardTextOnly;

public:
    CardFrame(int width, int height, const QString &cardName = QString(),
                QWidget *parent = 0);
    void setCardTextOnly(bool status) { cardTextOnly = status; hasPictureChanged(); }

public slots:
    void setCard(CardInfo *card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);
    void clear();

private slots:
    void hasPictureChanged();
    void toggleCardTextOnly() { setCardTextOnly(!cardTextOnly); }
};

#endif
