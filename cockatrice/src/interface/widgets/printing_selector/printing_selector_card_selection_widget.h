/**
 * @file printing_selector_card_selection_widget.h
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H
#define PRINTING_SELECTOR_CARD_SELECTION_WIDGET_H

#include <QWidget>
#include <qtmetamacros.h>

class DeckStateManager;
class PrintingSelector;
class QHBoxLayout;
class QPushButton;

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
