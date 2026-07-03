#ifndef TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H
#define TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H

#include "../../interface/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../interface/widgets/printing_selector/printing_selector.h"
#include "../../interface/widgets/visual_database_display/visual_database_display_widget.h"
#include "../../interface/widgets/visual_deck_editor/visual_deck_editor_sample_hand_widget.h"
#include "../../interface/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../abstract_tab_deck_editor.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @class TabDeckEditorVisualTabWidget
 * @ingroup DeckEditorWidgets
 * @brief Tab container for the visual deck editor.
 *
 * **Description:**
 * TabDeckEditorVisualTabWidget is a QTabWidget container for visual deck editing sub-tabs. It organizes the visual
 * deck, database display, deck analytics, and sample hand preview into separate tabs.
 *
 * **Purpose:**
 * - Provides a tabbed interface for multiple visual components of the deck editor.
 * - Handles forwarding of card events (hover, click, selection) to TabDeckEditorVisual.
 * - Manages dynamic tab addition, removal, and title updates.
 *
 * **Contained Sub-Tabs and Widgets:**
 * - VisualDeckEditorWidget — Visual deck interface with card interactions.
 * - VisualDatabaseDisplayWidget — Displays card database visually and supports interactions.
 * - DeckAnalyticsWidget — Shows deck statistics and metrics.
 * - VisualDeckEditorSampleHandWidget — Simulates and displays sample hands.
 *
 * **Key Methods:**
 * - addNewTab(QWidget *widget, const QString &title) — Adds a new tab to the container.
 * - removeCurrentTab() — Removes the currently selected tab.
 * - setTabTitle(int index, const QString &title) — Sets the title of a specific tab.
 * - getCurrentTab() const — Returns the currently active tab widget.
 * - getTabCount() const — Returns the number of tabs.
 * - handleTabClose(int index) — Handles tab close requests and deletes the widget.
 * - onCardChanged(const ExactCard &activeCard) — Emits card change signal from visual deck.
 * - onCardChangedDatabaseDisplay(const ExactCard &activeCard) — Emits card change signal from database display.
 * - onCardClickedDeckEditor(QMouseEvent *event, ...) — Emits click events from deck editor visual tab.
 * - onCardClickedDatabaseDisplay(QMouseEvent *event, ...) — Emits click events from database display tab.
 */
class TabDeckEditorVisualTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct the tab widget with required models.
     * @param parent Parent widget.
     * @param _deckEditor Pointer to the deck editor instance.
     * @param _deckModel Deck list model.
     * @param _cardDatabaseModel Card database model.
     */
    explicit TabDeckEditorVisualTabWidget(QWidget *parent,
                                          AbstractTabDeckEditor *_deckEditor,
                                          DeckListModel *_deckModel,
                                          CardDatabaseModel *_cardDatabaseModel);

    /** @brief Add a new tab with a widget and title. */
    void addNewTab(QWidget *widget, const QString &title);

    /** @brief Remove the currently active tab. */
    void removeCurrentTab();

    /** @brief Set the title of a specific tab. */
    void setTabTitle(int index, const QString &title);

    /** @brief Get the currently active tab widget. */
    [[nodiscard]] QWidget *getCurrentTab() const;

    /** @brief Get the total number of tabs. */
    [[nodiscard]] int getTabCount() const;

    VisualDeckEditorWidget *visualDeckView; ///< Visual deck editor widget.
    DeckListStatisticsAnalyzer *statsAnalyzer;
    DeckAnalyticsWidget *deckAnalytics;                 ///< Deck analytics widget.
    VisualDatabaseDisplayWidget *visualDatabaseDisplay; ///< Database display widget.
    PrintingSelector *printingSelector;                 ///< Printing selector widget.
    VisualDeckEditorSampleHandWidget *sampleHandWidget; ///< Sample hand simulation widget.

public slots:
    /**
     * @brief Emitted when the active card changes in the deck view.
     * @param activeCard New active card.
     */
    void onCardChanged(const ExactCard &activeCard);

    /**
     * @brief Emitted when the active card changes in the database display.
     * @param activeCard New active card.
     */
    void onCardChangedDatabaseDisplay(const ExactCard &activeCard);

    /**
     * @brief Emitted when a card is clicked in the deck view.
     * @param event Mouse event.
     * @param card The clicked card.
     * @param zoneName Deck zone of the card.
     */
    void onCardClickedDeckEditor(QMouseEvent *event, const ExactCard &card, const QString &zoneName);

    /**
     * @brief Emitted when a card is clicked in the database display.
     * @param event Mouse event.
     * @param card The clicked card.
     */
    void onCardClickedDatabaseDisplay(QMouseEvent *event, const ExactCard &card);

signals:
    void cardChanged(const ExactCard &activeCard);
    void cardChangedDatabaseDisplay(const ExactCard &activeCard);
    void cardClicked(QMouseEvent *event, const ExactCard &card, const QString &zoneName);
    void cardClickedDatabaseDisplay(QMouseEvent *event, const ExactCard &card);

    void cardAdded(const ExactCard &card, const QString &zoneName);
    void cardDecremented(const ExactCard &card, const QString &zoneName);
    void edhrecRequested(const CardInfoPtr &cardInfo, bool isCommander);
    void printingSelectorRequested();
    void cardInfoRequested(const ExactCard &cardName);

private:
    QVBoxLayout *layout;                  ///< Layout for tabs and controls.
    AbstractTabDeckEditor *deckEditor;    ///< Reference to the deck editor.
    DeckListModel *deckModel;             ///< Deck list model.
    CardDatabaseModel *cardDatabaseModel; ///< Card database model.

private slots:
    /**
     * @brief Handle closing of a tab at a given index.
     * @param index Index of the tab to close.
     */
    void handleTabClose(int index);

    /**
     * @brief Adds card to maindeck or side depending on whether ctrl is held
     * @param card
     */
    void actAddCard(const ExactCard &card);
};

#endif // TAB_DECK_EDITOR_VISUAL_TAB_WIDGET_H