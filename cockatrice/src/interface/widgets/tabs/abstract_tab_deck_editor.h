/**
 * @file abstract_tab_deck_editor.h
 * @brief Defines the AbstractTabDeckEditor class, which provides a base for
 * deck editor tabs in the application.
 *
 */

#ifndef TAB_GENERIC_DECK_EDITOR_H
#define TAB_GENERIC_DECK_EDITOR_H

#include "../interface/widgets/deck_editor/deck_editor_card_info_dock_widget.h"
#include "../interface/widgets/deck_editor/deck_editor_database_display_widget.h"
#include "../interface/widgets/deck_editor/deck_editor_deck_dock_widget.h"
#include "../interface/widgets/deck_editor/deck_editor_filter_dock_widget.h"
#include "../interface/widgets/deck_editor/deck_editor_printing_selector_dock_widget.h"
#include "../interface/widgets/menus/deck_editor_menu.h"
#include "../interface/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "tab.h"

class CardDatabaseModel;
class CardDatabaseDisplayModel;

class CardInfoFrameWidget;
class DeckLoader;
class DeckEditorMenu;
class DeckEditorCardInfoDockWidget;
class DeckEditorDatabaseDisplayWidget;
class DeckEditorDeckDockWidget;
class DeckEditorFilterDockWidget;
class DeckEditorPrintingSelectorDockWidget;
class DeckPreviewDeckTagsDisplayWidget;
class Response;
class FilterTree;
class FilterTreeModel;
class FilterBuilder;

class QTreeView;
class QTextEdit;
class QLabel;
class QComboBox;
class QGroupBox;
class QMessageBox;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QDockWidget;
class QMenu;
class QAction;

/**
 * @class AbstractTabDeckEditor
 * @ingroup DeckEditorTabs
 * @brief AbstractTabDeckEditor is the base class for all deck editor tabs.
 *
 * **Description:**
 * AbstractTabDeckEditor is the base class for all deck editor tabs. It provides core functionality such as deck model
 * management, card addition/removal, and integration with dock widgets and tab supervisors.
 *
 * **Purpose:**
 *
 * - Acts as the foundation for deck editor tabs (TabDeckEditor and TabDeckEditorVisual).
 * - Provides basic deck operations like adding, removing, swapping, and decrementing cards.
 * - Integrates with DeckListModel and CardDatabaseModel to access deck and card data.
 * - Handles saving, loading, and layout persistence at the tab level.
 *
 * **Dock Widgets (typically managed in derived classes):**
 *
 * - DeckEditorCardInfoDockWidget — Displays detailed card info.
 * - DeckEditorDeckDockWidget— Displays mainboard/sideboard cards and zones.
 * - DeckEditorFilterDockWidget— Provides filtering options for card searches.
 * - DeckEditorPrintingSelectorDockWidget— Selector for different card printings.
 * - DeckEditorDatabaseDisplayWidget— Shows card database for adding cards to deck.
 *
 * **Key Methods:**
 *
 * - actAddCard(const ExactCard &card) — Adds a card to the deck.
 * - actDecrementCard(const ExactCard &card) — Removes a single instance of a card from the deck.
 * - actSwapCard(const ExactCard &card, const QString &zone) — Swaps a card between zones.
 * - actRemoveCard() — Removes the currently selected card from the deck.
 * - actSaveDeckAs() — Performs a "Save As" action for the deck.
 * - updateCard(const ExactCard &card) — Updates the currently displayed card info in the dock.
 * - onDeckModified() — Called when the deck model is changed.
 *
 * Provides UI docks for the deck, database, card info, printing selector,
 * and filters. Supports loading, saving, editing, exporting decks, and
 * interactions with external services such as DeckStats, TappedOut, and
 * remote deck uploads.
 */
class AbstractTabDeckEditor : public Tab
{
    Q_OBJECT

    friend class DeckEditorMenu;

public:
    /**
     * @brief Constructs an AbstractTabDeckEditor.
     * @param _tabSupervisor Pointer to the TabSupervisor managing this tab.
     */
    explicit AbstractTabDeckEditor(TabSupervisor *_tabSupervisor);

    /** @brief Creates the menus for this tab. Pure virtual. */
    virtual void createMenus() = 0;

    /** @brief Returns the display text for the tab. */
    [[nodiscard]] virtual QString getTabText() const override = 0;

    /** @brief Confirms whether the tab can be safely closed. */
    bool confirmClose();

    /** @brief Retranslates the UI text. Pure virtual. */
    virtual void retranslateUi() override = 0;

    /** @brief Opens a deck in this tab.
     *  @param deck Pointer to a DeckLoader object.
     */
    void openDeck(DeckLoader *deck);

    /** @brief Returns the currently active deck loader. */
    DeckLoader *getDeckLoader() const;

    /** @brief Returns the currently active deck list. */
    DeckList *getDeckList() const;

    /** @brief Sets the modified state of the tab.
     *  @param _windowModified Whether the tab is modified.
     */
    void setModified(bool _windowModified);

    DeckEditorDeckDockWidget *getDeckDockWidget() const
    {
        return deckDockWidget;
    }

    // UI Elements
    DeckEditorMenu *deckMenu;                                         ///< Menu for deck operations
    DeckEditorDatabaseDisplayWidget *databaseDisplayDockWidget;       ///< Database dock
    DeckEditorCardInfoDockWidget *cardInfoDockWidget;                 ///< Card info dock
    DeckEditorDeckDockWidget *deckDockWidget;                         ///< Deck dock
    DeckEditorFilterDockWidget *filterDockWidget;                     ///< Filter dock
    DeckEditorPrintingSelectorDockWidget *printingSelectorDockWidget; ///< Printing selector dock

public slots:
    /** @brief Called when the deck changes. */
    virtual void onDeckChanged();

    /** @brief Called when the deck is modified. */
    virtual void onDeckModified();

    /** @brief Updates the card info panel.
     *  @param card The card to display.
     */
    void updateCard(const ExactCard &card);

    /** @brief Adds a card to the main deck or sideboard based on Ctrl key. */
    void actAddCard(const ExactCard &card);

    /** @brief Adds a card to the sideboard explicitly. */
    void actAddCardToSideboard(const ExactCard &card);

    /** @brief Decrements a card from the main deck. */
    void actDecrementCard(const ExactCard &card);

    /** @brief Decrements a card from the sideboard. */
    void actDecrementCardFromSideboard(const ExactCard &card);

    /** @brief Opens a recently opened deck file. */
    void actOpenRecent(const QString &fileName);

    /** @brief Called when the filter tree changes. */
    void filterTreeChanged(FilterTree *filterTree);

    /** @brief Requests closing the tab. */
    bool closeRequest() override;

    /** @brief Shows the printing selector dock. Pure virtual. */
    virtual void showPrintingSelector() = 0;

    /** @brief Slot for when a dock's top-level state changes. Pure virtual. */
    virtual void dockTopLevelChanged(bool topLevel) = 0;

signals:
    /** @brief Emitted when a deck should be opened in a new editor tab. */
    void openDeckEditor(DeckLoader *deckLoader);

    /** @brief Emitted before the tab is closed. */
    void deckEditorClosing(AbstractTabDeckEditor *tab);

    /** @brief Emitted when a card should be decremented. */
    void decrementCard(const ExactCard &card, QString zoneName);

protected slots:
    /** @brief Starts a new deck in this tab. */
    virtual void actNewDeck();

    /** @brief Cleans the current deck and resets the modified state. */
    void cleanDeckAndResetModified();

    /** @brief Loads a deck from file. */
    virtual void actLoadDeck();

    /** @brief Saves the current deck. */
    bool actSaveDeck();

    /** @brief Saves the current deck under a new name. */
    virtual bool actSaveDeckAs();

    /** @brief Loads a deck from the clipboard. */
    virtual void actLoadDeckFromClipboard();

    /** @brief Opens a deck editor for clipboard contents. */
    void actEditDeckInClipboard();

    /** @brief Opens a raw clipboard deck editor. */
    void actEditDeckInClipboardRaw();

    /** @brief Saves deck to clipboard with full info. */
    void actSaveDeckToClipboard();

    /** @brief Saves deck to clipboard without set info. */
    void actSaveDeckToClipboardNoSetInfo();

    /** @brief Saves deck to clipboard in raw format. */
    void actSaveDeckToClipboardRaw();

    /** @brief Saves deck to clipboard in raw format without set info. */
    void actSaveDeckToClipboardRawNoSetInfo();

    /** @brief Prints the deck using a preview dialog. */
    void actPrintDeck();

    /** @brief Loads a deck from an online website. */
    void actLoadDeckFromWebsite();

    /** @brief Exports the deck to decklist.org. */
    void actExportDeckDecklist();

    /** @brief Exports the deck to decklist.xyz. */
    void actExportDeckDecklistXyz();

    /** @brief Analyzes the deck using deckstats.net. */
    void actAnalyzeDeckDeckstats();

    /** @brief Analyzes the deck using tappedout.net. */
    void actAnalyzeDeckTappedout();

    /** @brief Callback when a remote deck save finishes. */
    void saveDeckRemoteFinished(const Response &r);

    // UI Layout Management
    virtual void loadLayout() = 0;
    virtual void restartLayout() = 0;
    virtual void freeDocksSize() = 0;
    virtual void refreshShortcuts() = 0;

    /** @brief Handles dock close events. */
    void closeEvent(QCloseEvent *event) override;

    /** @brief Event filter for dock state changes. */
    bool eventFilter(QObject *o, QEvent *e) override;

    /** @brief Slot triggered when a dock visibility changes. Pure virtual. */
    virtual void dockVisibleTriggered() = 0;

    /** @brief Slot triggered when a dock floating state changes. Pure virtual. */
    virtual void dockFloatingTriggered() = 0;

private:
    /** @brief Sets the deck for this tab.
     *  @param _deck The deck object.
     */
    virtual void setDeck(DeckLoader *_deck);

    /** @brief Helper for editing decks from the clipboard. */
    void editDeckInClipboard(bool annotated);

    /** @brief Helper for exporting decks to websites.
     *  @param website Target website.
     */
    void exportToDecklistWebsite(DeckLoader::DecklistWebsite website);

protected:
    /** @brief Enum describing deck open locations */
    enum DeckOpenLocation
    {
        CANCELLED, ///< Operation cancelled
        SAME_TAB,  ///< Open deck in the same tab
        NEW_TAB    ///< Open deck in a new tab
    };

    /** @brief Confirms deck open action based on settings and modified state.
     *  @param openInSameTabIfBlank Whether to reuse same tab if blank.
     *  @return Selected DeckOpenLocation.
     */
    DeckOpenLocation confirmOpen(bool openInSameTabIfBlank = true);

    /** @brief Creates a save confirmation message box.
     *  @return Pointer to a QMessageBox.
     */
    QMessageBox *createSaveConfirmationWindow();

    /** @brief Returns true if the tab is a blank newly created deck. */
    bool isBlankNewDeck() const;

    /** @brief Helper function to add a card to a specific deck zone. */
    void addCardHelper(const ExactCard &card, QString zoneName);

    /** @brief Swaps a card in the deck view. */
    void actSwapCard(const ExactCard &card, const QString &zoneName);

    /** @brief Opens a deck from a file. */
    virtual void openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation);

    // UI Menu Elements
    QMenu *viewMenu, *cardInfoDockMenu, *deckDockMenu, *filterDockMenu, *printingSelectorDockMenu;

    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating;
    QAction *aFilterDockVisible, *aFilterDockFloating, *aPrintingSelectorDockVisible, *aPrintingSelectorDockFloating;

    bool modified = false; ///< Whether the deck/tab has unsaved changes
};

#endif // TAB_GENERIC_DECK_EDITOR_H
