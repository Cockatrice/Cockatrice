#ifndef PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
#define PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_info_picture.h"

#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class PrintingSelectorCardDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    PrintingSelectorCardDisplayWidget(DeckListModel *deckModel,
                                      QTreeView *deckView,
                                      CardInfoPtr &rootCard,
                                      CardInfoPerSet &setInfoForCard,
                                      QWidget *parent = nullptr);
    int countCards();

private:
    QVBoxLayout *layout;
    QHBoxLayout *buttonBox;
    QPushButton *incrementButton;
    QPushButton *decrementButton;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr rootCard;
    CardInfoPtr setCard;
    CardInfoPerSet setInfoForCard;

    CardInfoPicture *cardInfoPicture;
    QLabel *cardCount;
    QLabel *setName;
    QLabel *setNumber;

    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(QString zoneName);

private slots:
    void addPrinting();
    void removePrinting();
};

#endif // PRINTING_SELECTOR_CARD_DISPLAY_WIDGET_H
