#ifndef CARDFRAME_H
#define CARDFRAME_H

#include <QTabWidget>

class AbstractCardItem;
class CardInfo;
class CardInfoPicture;
class CardInfoText;
class QVBoxLayout;

class CardFrame : public QTabWidget {
    Q_OBJECT

private:
    CardInfo *info;
    CardInfoPicture *pic;
    CardInfoText *text;
    bool cardTextOnly;
    QWidget *tab1, *tab2, *tab3;
    QVBoxLayout *tab1Layout, *tab2Layout, *tab3Layout;

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
