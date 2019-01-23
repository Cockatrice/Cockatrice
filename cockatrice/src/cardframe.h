#ifndef CARDFRAME_H
#define CARDFRAME_H

#include <QTabWidget>

#include "carddatabase.h"

class AbstractCardItem;
class CardInfoPicture;
class CardInfoText;
class QVBoxLayout;
class QSplitter;

class CardFrame : public QTabWidget
{
    Q_OBJECT
private:
    CardInfoPtr info;
    CardInfoPicture *pic;
    CardInfoText *text;
    bool cardTextOnly;
    QWidget *tab1, *tab2, *tab3;
    QVBoxLayout *tab1Layout, *tab2Layout, *tab3Layout;
    QSplitter *splitter;

public:
    enum ViewMode
    {
        ImageOnlyView,
        TextOnlyView,
        ImageAndTextView
    };
    explicit CardFrame(const QString &cardName = QString(), QWidget *parent = nullptr);
    void retranslateUi();

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const QString &cardName);
    void setCard(AbstractCardItem *card);
    void clearCard();
    void setViewMode(int mode);
};

#endif
