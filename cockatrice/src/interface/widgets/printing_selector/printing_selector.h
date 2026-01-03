/**
 * @file printing_selector.h
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef PRINTING_SELECTOR_H
#define PRINTING_SELECTOR_H

#include "../cards/card_size_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "../quick_settings/settings_button_widget.h"

#include <QCheckBox>
#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

#define BATCH_SIZE 10

class DeckStateManager;
class PrintingSelectorCardSearchWidget;
class PrintingSelectorCardSelectionWidget;
class PrintingSelectorCardSortingWidget;
class PrintingSelectorViewOptionsWidget;
class AbstractTabDeckEditor;
class PrintingSelector : public QWidget
{
    Q_OBJECT

public:
    PrintingSelector(QWidget *parent, AbstractTabDeckEditor *deckEditor);

    void setCard(const CardInfoPtr &newCard);
    void getAllSetsForCurrentCard();

public slots:
    void retranslateUi();
    void updateDisplay();
    void toggleVisibilityNavigationButtons(bool _state);

private slots:
    void printingsInDeckChanged();
    void updateCardAmounts();

signals:
    /**
     * Requests the previous card in the list
     */
    void prevCardRequested();
    /**
     * Requests the next card in the list
     */
    void nextCardRequested();

    /**
     * The amounts of the printings in the deck has changed
     * @param uuidToAmounts Map of uuids to the amounts (maindeck, sideboard) in the deck
     */
    void cardAmountsChanged(const QMap<QString, QPair<int, int>> &uuidToAmounts);

private:
    QVBoxLayout *layout;
    SettingsButtonWidget *displayOptionsWidget;
    QWidget *sortAndOptionsContainer;
    QHBoxLayout *sortAndOptionsLayout;
    QCheckBox *navigationCheckBox;
    PrintingSelectorCardSortingWidget *sortToolBar;
    PrintingSelectorCardSearchWidget *searchBar;
    FlowWidget *flowWidget;
    CardSizeWidget *cardSizeWidget;
    PrintingSelectorCardSelectionWidget *cardSelectionBar;
    AbstractTabDeckEditor *deckEditor;
    DeckStateManager *deckStateManager;
    CardInfoPtr selectedCard;
    QTimer *widgetLoadingBufferTimer;
    int currentIndex = 0;
};

#endif // PRINTING_SELECTOR_H
