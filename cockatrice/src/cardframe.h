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

public:
    enum ViewMode
    {
        ImageOnlyView,
        TextOnlyView,
        ImageAndTextView
    };
    struct CardImageData
    {
        CardInfoPtr cardInfoPtr;
        QString cardSetCode;
        QString cardCollectorNumber;
    };

    explicit CardFrame(const QString &cardName = QString(), QWidget *parent = nullptr);
    void retranslateUi();
    [[nodiscard]] CardImageData getCardMetadata() const
    {
        if (cardVersionSelector) {
            return cardVersionSelector->currentData().value<CardImageData>();
        }
        return {};
    }
    [[nodiscard]] QComboBox *getCardVersionSelector() const
    {
        return cardVersionSelector;
    }

signals:
    void refreshCardVersion() const;

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const QString &cardName, CardImageData cardMetaData = {});
    void setCard(AbstractCardItem *card);
    void setCard(const CardImageData &cardImageData);
    void clearCard();
    void setViewMode(int mode);
    void updateCardImage(int newIndex);
};

#endif
