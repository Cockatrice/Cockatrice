#ifndef TAB_GENERIC_DECK_EDITOR_H
#define TAB_GENERIC_DECK_EDITOR_H

#include "../../game/cards/card_database.h"
#include "../menus/deck_editor/deck_editor_menu.h"
#include "../ui/widgets/deck_editor/deck_editor_card_info_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_database_display_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_deck_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_filter_dock_widget.h"
#include "../ui/widgets/deck_editor/deck_editor_printing_selector_dock_widget.h"
#include "../ui/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "tab.h"

#include <QDir>

class CardDatabaseModel;
class CardDatabaseDisplayModel;

class QTreeView;

class DeckEditorMenu;

class CardInfoFrameWidget;
class QTextEdit;
class QLabel;
class DeckLoader;
class DeckEditorCardInfoDockWidget;
class DeckEditorDatabaseDisplayWidget;
class DeckEditorDeckDockWidget;
class DeckEditorFilterDockWidget;
class DeckEditorPrintingSelectorDockWidget;
class DeckPreviewDeckTagsDisplayWidget;
class Response;
class FilterTreeModel;
class FilterBuilder;
class QComboBox;
class QGroupBox;
class QMessageBox;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QDockWidget;

class TabGenericDeckEditor : public Tab
{
    Q_OBJECT
protected slots:
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);
    void updatePrintingSelectorDatabase(const QModelIndex &current, const QModelIndex &previous);
    void updatePrintingSelectorDeckView(const QModelIndex &current, const QModelIndex &previous);
    void decklistCustomMenu(QPoint point);

    virtual void actNewDeck() = 0;
    virtual void actLoadDeck() = 0;
    bool actSaveDeck();
    bool actSaveDeckAs();
    virtual void actLoadDeckFromClipboard();
    void actSaveDeckToClipboard();
    void actSaveDeckToClipboardNoSetNameAndNumber();
    void actSaveDeckToClipboardRaw();
    void actSaveDeckToClipboardRawNoSetNameAndNumber();
    void actPrintDeck();
    void actExportDeckDecklist();
    void actAnalyzeDeckDeckstats();
    void actAnalyzeDeckTappedout();

    void saveDeckRemoteFinished(const Response &r);

    virtual void loadLayout() = 0;
    virtual void restartLayout() = 0;
    virtual void freeDocksSize() = 0;
    virtual void refreshShortcuts() = 0;

    bool eventFilter(QObject *o, QEvent *e) override;
    virtual void dockVisibleTriggered() = 0;
    virtual void dockFloatingTriggered() = 0;

protected:
    /**
     * @brief Which tab to open the new deck in
     */
    enum DeckOpenLocation
    {
        CANCELLED,
        SAME_TAB,
        NEW_TAB
    };
    DeckOpenLocation confirmOpen(const bool openInSameTabIfBlank = true);
    QMessageBox *createSaveConfirmationWindow();

    bool isBlankNewDeck() const;

    void decrementCardHelper(CardInfoPtr info, QString zoneName);
    void actSwapCard(CardInfoPtr info, QString zoneName);
    virtual void openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation) = 0;

    QMenu *viewMenu, *cardInfoDockMenu, *deckDockMenu, *filterDockMenu, *printingSelectorDockMenu;

    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating, *aFilterDockVisible,
        *aFilterDockFloating, *aPrintingSelectorDockVisible, *aPrintingSelectorDockFloating;

    bool modified;

public:
    explicit TabGenericDeckEditor(TabSupervisor *_tabSupervisor);
    virtual void retranslateUi() override = 0;
    [[nodiscard]] virtual QString getTabText() const override = 0;
    virtual void setDeck(DeckLoader *_deckLoader);
    void setModified(bool _windowModified);
    bool confirmClose();
    virtual void createMenus() = 0;
    void updateCardInfo(CardInfoPtr _card);
    void addCardHelper(CardInfoPtr info, QString zoneName);
    void actAddCardFromDatabase();
    DeckEditorMenu *deckMenu;
    DeckEditorDatabaseDisplayWidget *databaseDisplayDockWidget;
    DeckEditorCardInfoDockWidget *cardInfoDockWidget;
    DeckEditorDeckDockWidget *deckDockWidget;
    DeckEditorFilterDockWidget *filterDockWidget;
    DeckEditorPrintingSelectorDockWidget *printingSelectorDockWidget;

public slots:
    void actAddCard(CardInfoPtr info);
    void actAddCardToSideboardFromDatabase();
    void actAddCardToSideboard(CardInfoPtr info);
    void actDecrementCard(CardInfoPtr info);
    void actDecrementCardFromDatabase();
    void actDecrementCardFromSideboard(CardInfoPtr info);
    void actDecrementCardFromSideboardFromDatabase();
    void actOpenRecent(const QString &fileName);

    void filterTreeChanged(FilterTree *filterTree);

    void closeRequest(bool forced = false) override;
    virtual void showPrintingSelector() = 0;
    virtual void dockTopLevelChanged(bool topLevel) = 0;
signals:
    void openDeckEditor(const DeckLoader *deckLoader);
    void deckEditorClosing(TabGenericDeckEditor *tab);
};

#endif // TAB_GENERIC_DECK_EDITOR_H
