#ifndef WINDOW_DECKEDITOR_H
#define WINDOW_DECKEDITOR_H

#include "../../deck/custom_line_edit.h"
#include "../../game/cards/card_database.h"
#include "../game_logic/key_signals.h"
#include "../ui/widgets/printing_selector/printing_selector.h"
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

class TabDeckEditor : public Tab
{
    Q_OBJECT
private slots:
    void updateName(const QString &name);
    void updateComments();
    void updateBannerCardComboBox();
    void setBannerCard(int);
    void updateHash();
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);
    void updatePrintingSelectorDatabase(const QModelIndex &current, const QModelIndex &previous);
    void updatePrintingSelectorDeckView(const QModelIndex &current, const QModelIndex &previous);
    void updateSearch(const QString &search);
    void databaseCustomMenu(QPoint point);
    void decklistCustomMenu(QPoint point);
    void updateRecentlyOpened();

    void actNewDeck();
    void actLoadDeck();
    void actOpenRecent(const QString &fileName);
    void actClearRecents();
    bool actSaveDeck();
    bool actSaveDeckAs();
    void actLoadDeckFromClipboard();
    void actSaveDeckToClipboard();
    void actSaveDeckToClipboardRaw();
    void actPrintDeck();
    void actExportDeckDecklist();
    void actAnalyzeDeckDeckstats();
    void actAnalyzeDeckTappedout();

    void actClearFilterAll();
    void actClearFilterOne();

    void actSwapCard();
    void actAddCard();
    void actAddCardToSideboard();
    void actRemoveCard();
    void actIncrement();
    void actDecrement();
    void actDecrementCard();
    void actDecrementCardFromSideboard();
    void copyDatabaseCellContents();

    void saveDeckRemoteFinished(const Response &r);
    void filterViewCustomContextMenu(const QPoint &point);
    void filterRemove(QAction *action);

    void loadLayout();
    void restartLayout();
    void freeDocksSize();
    void refreshShortcuts();

    bool eventFilter(QObject *o, QEvent *e) override;
    void dockVisibleTriggered();
    void dockFloatingTriggered();
    void dockTopLevelChanged(bool topLevel);
    void saveDbHeaderState();
    void setSaveStatus(bool newStatus);
    void showSearchSyntaxHelp();

private:
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
    CardInfoPtr currentCardInfo() const;
    void addCardHelper(QString zoneName);
    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(QString zoneName);
    bool swapCard(const QModelIndex &idx);
    void recursiveExpand(const QModelIndex &index);
    void openDeckFromFile(const QString &fileName, DeckOpenLocation deckOpenLocation);

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
    QLabel *hashLabel1;
    LineEditUnfocusable *hashLabel;
    FilterTreeModel *filterModel;
    QTreeView *filterView;
    KeySignals filterViewKeySignals;
    QWidget *filterBox;

    QMenu *deckMenu, *viewMenu, *cardInfoDockMenu, *deckDockMenu, *filterDockMenu, *printingSelectorDockMenu,
        *analyzeDeckMenu, *saveDeckToClipboardMenu, *loadRecentDeckMenu;
    QAction *aNewDeck, *aLoadDeck, *aClearRecents, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard,
        *aSaveDeckToClipboard, *aSaveDeckToClipboardRaw, *aPrintDeck, *aExportDeckDecklist, *aAnalyzeDeckDeckstats,
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
    explicit TabDeckEditor(TabSupervisor *_tabSupervisor, QWidget *parent = nullptr);
    void retranslateUi() override;
    QString getTabText() const override;
    void setDeck(DeckLoader *_deckLoader);
    void setModified(bool _windowModified);
    bool confirmClose();
    void createDeckDock();
    void createCardInfoDock();
    void createFiltersDock();
    void createPrintingSelectorDock();
    void createMenus();
    void createCentralFrame();
    void updateCardInfo(CardInfoPtr _card);

public slots:
    void closeRequest(bool forced = false) override;
    void showPrintingSelector();
signals:
    void openDeckEditor(const DeckLoader *deckLoader);
    void deckEditorClosing(TabDeckEditor *tab);
};

#endif
