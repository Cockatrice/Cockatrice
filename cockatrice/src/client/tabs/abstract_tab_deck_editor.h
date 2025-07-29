#ifndef TAB_GENERIC_DECK_EDITOR_H
#define TAB_GENERIC_DECK_EDITOR_H

#include "../../game/cards/card_info.h"
#include "../menus/deck_editor/deck_editor_menu.h"
#include "../ui/widgets/deck_editor/deck_editor_card_info_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_database_display_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_deck_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_filter_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_printing_selector_dock_widget.h"
#include "../ui/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
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

class AbstractTabDeckEditor : public Tab
{
    Q_OBJECT

    friend class DeckEditorMenu;

public:
    explicit AbstractTabDeckEditor(TabSupervisor *_tabSupervisor);

    // UI and Navigation
    virtual void createMenus() = 0;
    [[nodiscard]] virtual QString getTabText() const override = 0;
    bool confirmClose();
    virtual void retranslateUi() override = 0;

    // Deck Management
    void openDeck(DeckLoader *deck);
    DeckLoader *getDeckList() const;
    void setModified(bool _windowModified);

    // UI Elements
    DeckEditorMenu *deckMenu;
    DeckEditorDatabaseDisplayWidget *databaseDisplayDockWidget;
    DeckEditorCardInfoDockWidget *cardInfoDockWidget;
    DeckEditorDeckDockWidget *deckDockWidget;
    DeckEditorFilterDockWidget *filterDockWidget;
    DeckEditorPrintingSelectorDockWidget *printingSelectorDockWidget;

public slots:
    virtual void onDeckChanged();
    virtual void onDeckModified();
    void updateCard(const ExactCard &card);
    void actAddCard(const ExactCard &card);
    void actAddCardToSideboard(const ExactCard &card);
    void actDecrementCard(const ExactCard &card);
    void actDecrementCardFromSideboard(const ExactCard &card);
    void actOpenRecent(const QString &fileName);
    void filterTreeChanged(FilterTree *filterTree);
    void closeRequest(bool forced = false) override;
    virtual void showPrintingSelector() = 0;
    virtual void dockTopLevelChanged(bool topLevel) = 0;

signals:
    void openDeckEditor(const DeckLoader *deckLoader);
    void deckEditorClosing(AbstractTabDeckEditor *tab);
    void decrementCard(const ExactCard &card, QString zoneName);

protected slots:
    // Deck Operations
    virtual void actNewDeck();
    void cleanDeckAndResetModified();
    virtual void actLoadDeck();
    bool actSaveDeck();
    virtual bool actSaveDeckAs();
    virtual void actLoadDeckFromClipboard();
    void actEditDeckInClipboard();
    void actEditDeckInClipboardRaw();
    void actSaveDeckToClipboard();
    void actSaveDeckToClipboardNoSetInfo();
    void actSaveDeckToClipboardRaw();
    void actSaveDeckToClipboardRawNoSetInfo();
    void actPrintDeck();
    void actLoadDeckFromWebsite();
    void actExportDeckDecklist();
    void actExportDeckDecklistXyz();
    void actAnalyzeDeckDeckstats();
    void actAnalyzeDeckTappedout();

    // Remote Save
    void saveDeckRemoteFinished(const Response &r);

    // UI Layout Management
    virtual void loadLayout() = 0;
    virtual void restartLayout() = 0;
    virtual void freeDocksSize() = 0;
    virtual void refreshShortcuts() = 0;

    bool eventFilter(QObject *o, QEvent *e) override;
    virtual void dockVisibleTriggered() = 0;
    virtual void dockFloatingTriggered() = 0;

private:
    virtual void setDeck(DeckLoader *_deck);
    void editDeckInClipboard(bool annotated);
    void exportToDecklistWebsite(DeckLoader::DecklistWebsite website);

protected:
    /**
     * @brief Enum for selecting deck open location
     */
    enum DeckOpenLocation
    {
        CANCELLED,
        SAME_TAB,
        NEW_TAB
    };

    DeckOpenLocation confirmOpen(bool openInSameTabIfBlank = true);
    QMessageBox *createSaveConfirmationWindow();
    bool isBlankNewDeck() const;

    // Helper functions for card actions
    void addCardHelper(const ExactCard &card, QString zoneName);
    void actSwapCard(const ExactCard &card, const QString &zoneName);
    virtual void openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation);

    // UI Menu Elements
    QMenu *viewMenu, *cardInfoDockMenu, *deckDockMenu, *filterDockMenu, *printingSelectorDockMenu;

    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating;
    QAction *aFilterDockVisible, *aFilterDockFloating, *aPrintingSelectorDockVisible, *aPrintingSelectorDockFloating;

    bool modified = false;
};

#endif // TAB_GENERIC_DECK_EDITOR_H
