/**
 * @file printing_selector.h
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef PRINTING_SELECTOR_H
#define PRINTING_SELECTOR_H

#include "printing_selector_host.h"

#include <QList>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/printing/exact_card.h>
#include <qtmetamacros.h>

class CardSizeWidget;
class DeckStateManager;
class FlowWidget;
class PrintingSelectorCardDisplayWidget;
class PrintingSelectorPlaceholderWidget;
class QCheckBox;
class QHBoxLayout;
class QString;
class QTimer;
class QShowEvent;
class QHideEvent;
class QVBoxLayout;
class SettingsButtonWidget;
template <class Key, class T> class QMap;

#define BATCH_SIZE 10

struct ZoneCounts
{
    int mainboard = 0;
    int sideboard = 0;
    int tokensboard = 0;
};

class PrintingSelectorCardSearchWidget;
class PrintingSelectorCardSelectionWidget;
class PrintingSelectorCardSortingWidget;

class PrintingSelector : public QWidget
{
    Q_OBJECT

public:
    PrintingSelector(QWidget *parent, PrintingSelectorHost *host);

    void setCard(const CardInfoPtr &newCard);
    void setSelectedProviderId(const QString &providerId);
    void setPlaceholderText(const QString &text);
    void getAllSetsForCurrentCard();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

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
     * @param uuidToAmounts Map of uuids to the amounts (maindeck, sideboard, tokensboard) in the deck
     */
    void cardAmountsChanged(const QMap<QString, ZoneCounts> &uuidToAmounts);

    /**
     * A printing was picked while the selector runs in selection-only mode.
     * @param card The picked card and printing.
     */
    void printingSelected(const ExactCard &card);

private:
    QVBoxLayout *layout;
    SettingsButtonWidget *displayOptionsWidget;
    QWidget *sortAndOptionsContainer;
    QHBoxLayout *sortAndOptionsLayout;
    QCheckBox *navigationCheckBox = nullptr;
    PrintingSelectorCardSortingWidget *sortToolBar;
    PrintingSelectorCardSearchWidget *searchBar;
    PrintingSelectorPlaceholderWidget *placeholderWidget;
    FlowWidget *flowWidget;
    CardSizeWidget *cardSizeWidget;
    PrintingSelectorCardSelectionWidget *cardSelectionBar = nullptr;
    PrintingSelectorHost *host;
    DeckStateManager *deckStateManager;
    QList<PrintingSelectorCardDisplayWidget *> cardDisplayWidgets;
    CardInfoPtr selectedCard;
    QString selectedProviderId;
    QString placeholderText;
    QTimer *widgetLoadingBufferTimer;
    QTimer *deckChangeTimer;
    bool displayDirty = false;
    int currentIndex = 0;

    void updateSelectedCardWidgets() const;
};

#endif // PRINTING_SELECTOR_H
