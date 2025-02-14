#ifndef TAB_GENERIC_DECK_EDITOR_H
#define TAB_GENERIC_DECK_EDITOR_H

#include "../../deck/custom_line_edit.h"
#include "../../game/cards/card_database.h"
#include "../game_logic/key_signals.h"
#include "../ui/widgets/printing_selector/printing_selector.h"
#include "../ui/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.h"
#include "tab.h"

#include <QAbstractItemModel>
#include <QDir>

class CardDatabaseModel;
class CardDatabaseDisplayModel;
class DeckListModel;
class QTreeView;

class CardInfoFrameWidget;
class QTextEdit;
class QLabel;
class DeckLoader;
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
    void updateName(const QString &name);                                                         // generic
    void updateComments();                                                                        // generic
    void updateBannerCardComboBox();                                                              // generic
    void setBannerCard(int);                                                                      // generic
    void updateHash();                                                                            // generic
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);             // generic
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);            // generic
    void updatePrintingSelectorDatabase(const QModelIndex &current, const QModelIndex &previous); // possibly generic
    void updatePrintingSelectorDeckView(const QModelIndex &current, const QModelIndex &previous); // possibly generic
    void updateSearch(const QString &search);                                                     // generic
    void databaseCustomMenu(QPoint point);                                                        // generic
    void decklistCustomMenu(QPoint point);
    void updateRecentlyOpened(); // generic

    virtual void actNewDeck() = 0;
    virtual void actLoadDeck() = 0;                     // NOT generic
    void actOpenRecent(const QString &fileName);        // generic
    void actClearRecents();                             // generic
    bool actSaveDeck();                                 // generic
    bool actSaveDeckAs();                               // generic
    void actLoadDeckFromClipboard();                    // generic
    void actSaveDeckToClipboard();                      // generic
    void actSaveDeckToClipboardNoSetNameAndNumber();    // generic
    void actSaveDeckToClipboardRaw();                   // generic
    void actSaveDeckToClipboardRawNoSetNameAndNumber(); // generic
    void actPrintDeck();                                // generic
    void actExportDeckDecklist();                       // generic
    void actAnalyzeDeckDeckstats();                     // generic
    void actAnalyzeDeckTappedout();                     // generic

    void actClearFilterAll(); // generic
    void actClearFilterOne(); // generic

    void actSwapCard();                   // probably generic
    void actAddCard();                    // generic
    void actAddCardToSideboard();         // generic
    void actRemoveCard();                 // generic
    void actIncrement();                  // generic
    void actDecrement();                  // generic
    void actDecrementCard();              // generic
    void actDecrementCardFromSideboard(); // generic
    void copyDatabaseCellContents();      // generic

    void saveDeckRemoteFinished(const Response &r);        // generic
    void filterViewCustomContextMenu(const QPoint &point); // generic
    void filterRemove(QAction *action);                    // generic

    virtual void loadLayout() = 0;
    virtual void restartLayout() = 0; // somewhat generic
    virtual void freeDocksSize() = 0;
    virtual void refreshShortcuts() = 0;

    bool eventFilter(QObject *o, QEvent *e) override;
    virtual void dockVisibleTriggered() = 0;             // custom
    virtual void dockFloatingTriggered() = 0;            // custom
    virtual void dockTopLevelChanged(bool topLevel) = 0; // custom
    void saveDbHeaderState();                            // generic
    void setSaveStatus(bool newStatus);                  // generic
    void showSearchSyntaxHelp();                         // generic

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
    CardInfoPtr currentCardInfo() const;                         // generic
    void offsetCountAtIndex(const QModelIndex &idx, int offset); // generic
    void decrementCardHelper(QString zoneName);                  // generic
    bool swapCard(const QModelIndex &idx);                       // probably generic
    void recursiveExpand(const QModelIndex &index);              // generic
    virtual void openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation) = 0;

    QModelIndexList getSelectedCardNodes() const;

    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    DeckListModel *deckModel;
    QTreeView *databaseView;

    QTreeView *deckView;
    KeySignals deckViewKeySignals;
    CardInfoFrameWidget *cardInfo;
    PrintingSelector *printingSelector;
    SearchLineEdit *searchEdit;
    KeySignals searchKeySignals;

    QLabel *nameLabel;
    LineEditUnfocusable *nameEdit;
    QLabel *commentsLabel;
    QTextEdit *commentsEdit;
    QLabel *bannerCardLabel;
    QComboBox *bannerCardComboBox;
    DeckPreviewDeckTagsDisplayWidget *deckTagsDisplayWidget;
    QLabel *hashLabel1;
    LineEditUnfocusable *hashLabel;
    FilterTreeModel *filterModel;
    QTreeView *filterView;
    KeySignals filterViewKeySignals;
    QWidget *filterBox;

    QMenu *deckMenu, *viewMenu, *cardInfoDockMenu, *deckDockMenu, *filterDockMenu, *printingSelectorDockMenu,
        *analyzeDeckMenu, *saveDeckToClipboardMenu, *loadRecentDeckMenu;
    QAction *aNewDeck, *aLoadDeck, *aClearRecents, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard,
        *aSaveDeckToClipboard, *aSaveDeckToClipboardNoSetNameAndNumber, *aSaveDeckToClipboardRaw,
        *aSaveDeckToClipboardRawNoSetNameAndNumber, *aPrintDeck, *aExportDeckDecklist, *aAnalyzeDeckDeckstats,
        *aAnalyzeDeckTappedout, *aClose;
    QAction *aClearFilterAll, *aClearFilterOne;
    QAction *aAddCard, *aAddCardToSideboard, *aRemoveCard, *aIncrement, *aDecrement, *aSwapCard;
    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating, *aFilterDockVisible,
        *aFilterDockFloating, *aPrintingSelectorDockVisible, *aPrintingSelectorDockFloating;

    bool modified;
    QVBoxLayout *centralFrame;
    QHBoxLayout *searchLayout;
    QDockWidget *cardInfoDock;
    QDockWidget *deckDock;
    QDockWidget *filterDock;
    QDockWidget *printingSelectorDock;
    QWidget *centralWidget;

public:
    explicit TabGenericDeckEditor(TabSupervisor *_tabSupervisor);
    virtual void retranslateUi() override = 0;
    [[nodiscard]] virtual QString getTabText() const override = 0; // Subclasses MUST implement this
    void setDeck(DeckLoader *_deckLoader);                         // generic enough
    void setModified(bool _windowModified);                        // generic
    bool confirmClose();                                           // generic
    virtual void createDeckDock() = 0;
    virtual void createCardInfoDock() = 0;
    virtual void createFiltersDock() = 0; // generic
    virtual void createPrintingSelectorDock() = 0;
    virtual void createMenus() = 0;
    virtual void createCentralFrame() = 0;
    void updateCardInfo(CardInfoPtr _card);
    void addCardHelper(CardInfoPtr info, QString zoneName); // reasonably generic

public slots:
    void closeRequest(bool forced = false) override; // generic
    virtual void showPrintingSelector() = 0;
signals:
    void openDeckEditor(const DeckLoader *deckLoader);
    void deckEditorClosing(TabGenericDeckEditor *tab);
};

#endif // TAB_GENERIC_DECK_EDITOR_H
