#ifndef CARD_AMOUNT_WIDGET_H
#define CARD_AMOUNT_WIDGET_H

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../../../tabs/tab_deck_editor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTreeView>
#include <QWidget>

class CardAmountWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CardAmountWidget(QWidget *parent,
                              TabDeckEditor *deckEditor,
                              DeckListModel *deckModel,
                              QTreeView *deckView,
                              CardInfoPtr &rootCard,
                              CardInfoPerSet &setInfoForCard,
                              QString zoneName);
    int countCardsInZone(const QString &deckZone);

public slots:
    void updateCardCount();
    void addPrinting(const QString &zone);

protected:
    void paintEvent(QPaintEvent *event) override;
    void hideElements();
    void showElements();

private:
    TabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    CardInfoPtr rootCard;
    CardInfoPerSet setInfoForCard;
    QString zoneName;
    QHBoxLayout *layout;
    QPushButton *incrementButton;
    QPushButton *decrementButton;
    QLabel *cardCountInZone;

    bool hovered;
    QPropertyAnimation *fadeAnimation;

    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(const QString &zoneName);
    void recursiveExpand(const QModelIndex &index);

private slots:
    void addPrintingMainboard();
    void addPrintingSideboard();
    void removePrintingMainboard();
    void removePrintingSideboard();
};

#endif // CARD_AMOUNT_WIDGET_H