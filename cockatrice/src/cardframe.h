#ifndef CARDFRAME_H
#define CARDFRAME_H

#include <QFrame>

class AbstractCardItem;
class CardInfo;
class CardInfoPicture;
class CardInfoText;
class QTabBar;

class CardFrame : public QFrame {
    Q_OBJECT

private:
    QTabBar * tabBar;
    CardInfo *info;
    CardInfoPicture *pic;
    CardInfoText *text;
    bool cardTextOnly;

public:
    enum ViewMode { ImageOnlyView, TextOnlyView, ImageAndTextView };

    CardFrame(int width, int height, const QString &cardName = QString(),
                QWidget *parent = 0);
    void retranslateUi();

public slots:
    void setCard(CardInfo *card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);
    void clear();
    void setViewMode(int mode);
};

#endif
