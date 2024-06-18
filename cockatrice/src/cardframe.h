#ifndef CARDFRAME_H
#define CARDFRAME_H

#include "carddatabase.h"

#include <QComboBox>
#include <QTabWidget>

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
    QComboBox *cardVersionSelector;

    void refreshCardVersionSelector(const QString &cardName) const;

    struct CardImageData
    {
        CardInfoPtr cardInfoPtr;
        QString cardSetName;
        QString cardNumber;
    };

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
    void setCard(const CardImageData &cardImageData);
    void clearCard();
    void setViewMode(int mode);
    void updateCardImage(int newIndex);
};

#endif
