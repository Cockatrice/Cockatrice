#ifndef WINDOW_DECKEDITOR_H
#define WINDOW_DECKEDITOR_H

#include "../../key_signals.h"
#include "../interface/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "abstract_tab_deck_editor.h"

#include <libcockatrice/card/card_info.h>

class CardDatabaseModel;
class CardDatabaseDisplayModel;
class DeckListModel;

class QLabel;
class DeckLoader;

/**
 * @class TabDeckEditor
 * @ingroup DeckEditorTabs
 * @brief TabDeckEditor provides a fully-featured deck editor tab. It extends AbstractTabDeckEditor.
 *
 * **Description:**
 * TabDeckEditor is a fully-featured deck editor tab. It extends AbstractTabDeckEditor. It manages layout, dock
 * widgets, menus, and deck-specific actions for traditional (non-visual) editing.
 *
 * **Purpose:**
 *
 * - Provides a complete deck editing interface for mainboard, sideboard, and card database interactions.
 * - Handles dock widgets, keyboard shortcuts, layout persistence, and UI updates.
 * - Facilitates card addition, removal, and deck saving/loading through menu and UI interactions.
 *
 * **Dock Widgets:**
 *
 * - DeckEditorCardInfoDockWidget — Displays detailed card information.
 * - DeckEditorDeckDockWidget — Shows deck contents and zones.
 * - DeckEditorDatabaseDisplayWidget — Displays cards from the database in a table view for adding to the deck.
 * - DeckEditorFilterDockWidget — Provides filter/search capabilities for the database display.
 * - DeckEditorPrintingSelectorDockWidget — Allows selection of specific card printings.
 *
 * **Key Methods:**
 *
 * - loadLayout() — Loads saved layout or applies default layout positions and dock states.
 * - restartLayout() — Resets dock visibility, positions, and floating states to defaults.
 * - freeDocksSize() — Frees constraints on dock sizes to allow flexible resizing.
 * - refreshShortcuts() — Updates tab-specific shortcuts from settings.
 * - eventFilter(QObject *o, QEvent *e) — Handles dock close/hide events and saves layout state.
 * - dockVisibleTriggered() — Responds to menu actions toggling dock visibility.
 * - dockFloatingTriggered() — Responds to menu actions toggling dock floating state.
 * - dockTopLevelChanged(bool topLevel) — Updates menu states when a dock changes top-level/floating status.
 * - retranslateUi() — Updates all text/UI elements for localization.
 * - getTabText() const — Returns the tab title with a modified marker if applicable.
 * - createMenus() — Initializes menus for deck and view actions.
 * - showPrintingSelector() — Displays the printing selector dock for the current card.
 */
class TabDeckEditor : public AbstractTabDeckEditor
{
    Q_OBJECT

protected slots:
    /** @brief Loads the saved layout or default layout. */
    void loadLayout() override;

    /** @brief Resets the layout to default positions and dock states. */
    void restartLayout() override;

    /** @brief Frees the dock sizes for resizing flexibility. */
    void freeDocksSize() override;

    /** @brief Refreshes shortcuts for this tab from settings. */
    void refreshShortcuts() override;

    /** @brief Handles dock visibility, floating, and top-level changes. */
    bool eventFilter(QObject *o, QEvent *e) override;
    void dockVisibleTriggered() override;
    void dockFloatingTriggered() override;
    void dockTopLevelChanged(bool topLevel) override;

public:
    /**
     * @brief Constructs a TabDeckEditor instance.
     * @param _tabSupervisor Parent tab supervisor for managing tabs.
     */
    explicit TabDeckEditor(TabSupervisor *_tabSupervisor);

    /** @brief Retranslates UI elements for localization. */
    void retranslateUi() override;

    /** @brief Returns the tab text, including modified mark if applicable. */
    QString getTabText() const override;

    /** @brief Creates menus for deck editing and view options. */
    void createMenus() override;

public slots:
    /** @brief Shows the printing selector dock and updates it with current card. */
    void showPrintingSelector() override;
};

#endif
