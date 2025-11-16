#ifndef WINDOW_DECKEDITORVISUAL_H
#define WINDOW_DECKEDITORVISUAL_H

#include "../tab.h"
#include "tab_deck_editor_visual_tab_widget.h"

/**
 * @class TabDeckEditorVisual
 * @ingroup DeckEditorTabs
 * @brief TabDeckEditorVisual provides a fully-featured deck editor tab with an enhanced visual interface. It extends
 * AbstractTabDeckEditor.
 *
 * **Description:**
 * TabDeckEditorVisual is a fully-featured deck editor tab with an enhanced visual interface. It extends
 * AbstractTabDeckEditor and integrates multiple visual components such as a visual deck view, database display,
 * deck analytics, and sample hand preview.
 *
 * **Purpose:**
 *
 * - Provides an intuitive, visual interface for deck editing.
 * - Combines visual deck representation, card database browsing, and analytics into one tab.
 * - Supports card interactions via clicks, hover events, and drag-and-drop-like behavior (not yet).
 *
 * **Dock Widgets and Components:**
 *
 * - TabDeckEditorVisualTabWidget — Container for visual sub-tabs (further described in @ref
 *   code_client_tabs_deck_editor_visual_tab).
 *     - VisualDeckEditorWidget — Displays and interacts with the deck visually.
 *     - VisualDatabaseDisplayWidget — Allows adding cards from the database visually.
 *     - DeckAnalyticsWidget — Displays deck statistics and analytics.
 *     - VisualDeckEditorSampleHandWidget — Simulates a sample hand from the deck.
 * - DeckEditorCardInfoDockWidget — Shows detailed card info for hovered/selected cards.
 * - DeckEditorDeckDockWidget — Displays deck zones and cards in a tree-view.
 * - DeckEditorFilterDockWidget — Provides filtering options for card searches.
 * - DeckEditorPrintingSelectorDockWidget — Selects specific card printings.
 *
 * **Key Methods:**
 *
 * - createCentralFrame() — Sets up the central widget and visual sub-tabs.
 * - onDeckChanged() — Refreshes visual widgets when the deck is modified.
 * - changeModelIndexAndCardInfo(const ExactCard &card) — Updates deck model selection and card info.
 * - changeModelIndexToCard(const ExactCard &card) — Selects the card in the deck view.
 * - processMainboardCardClick(QMouseEvent *event, ...) — Handles clicks on mainboard cards.
 * - processCardClickDatabaseDisplay(QMouseEvent *event, ...) — Handles clicks on database cards.
 * - actSaveDeckAs() — Overrides save action with temporary UI adjustments.
 * - showPrintingSelector() — Opens the printing selector dock for the current card.
 * - freeDocksSize() — Frees constraints on dock widget sizes.
 * - refreshShortcuts() — Updates tab-specific shortcuts from settings.
 * - loadLayout() — Loads saved layout or applies default if none exists.
 * - restartLayout() — Resets dock positions, visibility, and floating states.
 * - retranslateUi() — Updates text/UI elements for localization.
 * - eventFilter(QObject *o, QEvent *e) — Syncs dock states with menu actions and saves layout.
 */
class TabDeckEditorVisual : public AbstractTabDeckEditor
{
    Q_OBJECT

protected slots:
    /**
     * @brief Load the editor layout from settings.
     */
    void loadLayout() override;

    /**
     * @brief Reset and restart the layout to default.
     */
    void restartLayout() override;

    /**
     * @brief Set size restrictions for free floating dock widgets.
     */
    void freeDocksSize() override;

    /**
     * @brief Refresh keyboard shortcuts for this tab.
     */
    void refreshShortcuts() override;

    /**
     * @brief Synchronize dock state with menu items.
     * @param o The object sending the event.
     * @param e The event being filtered.
     * @return true if the event is handled, false otherwise.
     */
    bool eventFilter(QObject *o, QEvent *e) override;

    /**
     * @brief Triggered when a dock visibility menu item is clicked.
     */
    void dockVisibleTriggered() override;

    /**
     * @brief Triggered when a dock floating menu item is clicked.
     */
    void dockFloatingTriggered() override;

    /**
     * @brief Triggered when a dock top-level state changes.
     * @param topLevel True if the dock became floating.
     */
    void dockTopLevelChanged(bool topLevel) override;

protected:
    TabDeckEditorVisualTabWidget *tabContainer; ///< Tab container holding different visual widgets.

    QVBoxLayout *centralFrame;           ///< Layout for central widgets.
    QVBoxLayout *searchAndDatabaseFrame; ///< Layout for search and database display.
    QHBoxLayout *searchLayout;           ///< Layout for search bar.
    QDockWidget *searchAndDatabaseDock;  ///< Dock widget for search/database display.
    QWidget *centralWidget;              ///< Central widget of the editor.

public:
    /**
     * @brief Constructs a visual deck editor tab.
     * @param _tabSupervisor Pointer to the tab supervisor managing this tab.
     */
    explicit TabDeckEditorVisual(TabSupervisor *_tabSupervisor);

    /**
     * @brief Retranslate UI strings (for i18n support).
     */
    void retranslateUi() override;

    /**
     * @brief Get the display text for the tab.
     * @return Tab text with optional modification indicator.
     */
    [[nodiscard]] QString getTabText() const override;

    /**
     * @brief Update the currently selected card in the deck and UI.
     * @param activeCard Card to display.
     */
    void changeModelIndexAndCardInfo(const ExactCard &activeCard);

    /**
     * @brief Change the deck view selection to a specific card.
     * @param activeCard Card to select in the deck view.
     */
    void changeModelIndexToCard(const ExactCard &activeCard);

    /**
     * @brief Create the deck analytics dock widget.
     */
    void createDeckAnalyticsDock();

    /**
     * @brief Setup menus for this visual deck editor.
     */
    void createMenus() override;

    /**
     * @brief Create search and database display frame.
     */
    void createSearchAndDatabaseFrame();

    /**
     * @brief Create central frame for visual widgets.
     */
    void createCentralFrame();

public slots:
    /**
     * @brief Refresh UI when the deck changes.
     */
    void onDeckChanged() override;

    /**
     * @brief Show the printing selector dock for the currently active card.
     */
    void showPrintingSelector() override;

    /**
     * @brief Handle card clicks in the mainboard visual deck.
     * @param event Mouse event triggering the action.
     * @param instance Widget representing the clicked card.
     * @param zoneName Deck zone of the card.
     */
    void
    processMainboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance, QString zoneName);

    /**
     * @brief Handle card clicks in the database visual display.
     * @param event Mouse event triggering the action.
     * @param instance Widget representing the clicked card.
     */
    void processCardClickDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);

    /**
     * @brief Save the deck under a new name.
     * @return true if successful, false otherwise.
     */
    bool actSaveDeckAs() override;
};

#endif