//
// Created by ascor on 11/6/24.
//

#ifndef PRINTING_SELECTOR_H
#define PRINTING_SELECTOR_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QTreeView>
#include <QWidget>

class PrintingSelector : public QWidget
{
    Q_OBJECT

public:
    PrintingSelector(DeckListModel *deckModel, QTreeView *deckView, CardInfoPtr &selectedCard, QWidget *parent = nullptr);
    CardInfoPerSet getSetForUUID(QString uuid);
    void getAllSetsForCurrentCard();
    void populateCards();

private:
    QHBoxLayout *layout;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr selectedCard;
};

#endif //PRINTING_SELECTOR_H
