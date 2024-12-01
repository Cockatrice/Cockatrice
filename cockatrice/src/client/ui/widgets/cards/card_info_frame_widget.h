#ifndef CARDFRAME_H
#define CARDFRAME_H

#include "../../../../game/cards/card_database.h"

#include <QTabWidget>

class AbstractCardItem;
class CardInfoPictureWidget;
class CardInfoTextWidget;
class QVBoxLayout;
class QSplitter;

class CardInfoFrameWidget : public QTabWidget
{
    Q_OBJECT
private:
    CardInfoPtr info;
    CardInfoPictureWidget *pic;
    CardInfoTextWidget *text;
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
    explicit CardInfoFrameWidget(const QString &cardName = QString(), QWidget *parent = nullptr);
    void retranslateUi();

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const QString &cardName);
    void setCard(const QString &cardName, const QString &providerId);
    void setCard(AbstractCardItem *card);
    void clearCard();
    void setViewMode(int mode);
};

#endif
