/**
 * @file printing_selector_card_selection_widget.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H
#define PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H

#include "printing_selector.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

class PrintingSelectorCardSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrintingSelectorCardSelectionWidget(PrintingSelector *parent, DeckStateManager *deckStateManager);

    void connectSignals();

public slots:
    void selectSetForCards();

private:
    PrintingSelector *parent;
    DeckStateManager *deckStateManager;
    QHBoxLayout *cardSelectionBarLayout;
    QPushButton *previousCardButton;
    QPushButton *selectSetForCardsButton;
    QPushButton *nextCardButton;
};

#endif // PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H
