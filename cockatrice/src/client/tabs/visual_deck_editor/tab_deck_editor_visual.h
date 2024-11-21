#ifndef WINDOW_DECKEDITORVISUAL_H
#define WINDOW_DECKEDITORVISUAL_H

#include "../../../deck/custom_line_edit.h"
#include "../../game_logic/key_signals.h"
#include "../tab.h"
#include "../tab_deck_editor.h"
#include "tab_deck_editor_visual_tab_widget.h"

#include <QAbstractItemModel>
#include <QDir>

class TabDeckEditorVisual : public Tab
{
    Q_OBJECT
private slots:
    void updateName(const QString &name);
    void updateComments();
    void updateHash();
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);
    void updateSearch(const QString &search);
    void databaseCustomMenu(QPoint point);

    void actNewDeck();
    void actLoadDeck();
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
    CardInfoPtr currentCardInfo() const;
    void changeModelIndexToCard(CardInfoPtr activeCard);
    void addCardHelper(QString zoneName);
    void addCardInfo(CardInfoPtr info, QString zoneName);
    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(QString zoneName);
    void recursiveExpand(const QModelIndex &index);

    QWidget *parent;

    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    DeckListModel *deckModel;
    QTreeView *databaseView;

    TabDeckEditorVisualTabWidget *tabContainer;

    QTreeView *deckView;
    KeySignals deckViewKeySignals;
    CardInfoFrameWidget *cardInfo;

    SearchLineEdit *searchEdit;
    KeySignals searchKeySignals;

    QLabel *nameLabel;
    LineEditUnfocusable *nameEdit;
    QLabel *commentsLabel;
    QTextEdit *commentsEdit;
    QLabel *hashLabel1;
    LineEditUnfocusable *hashLabel;
    FilterTreeModel *filterModel;
    QTreeView *filterView;
    KeySignals filterViewKeySignals;
    QWidget *filterBox;

    QMenu *deckMenu, *viewMenu, *cardInfoDockMenu, *deckDockMenu, *deckAnalyticsMenu, *filterDockMenu, *analyzeDeckMenu,
        *saveDeckToClipboardMenu;
    QAction *aNewDeck, *aLoadDeck, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard, *aSaveDeckToClipboard,
        *aSaveDeckToClipboardRaw, *aPrintDeck, *aExportDeckDecklist, *aAnalyzeDeckDeckstats, *aAnalyzeDeckTappedout,
        *aClose;
    QAction *aClearFilterAll, *aClearFilterOne;
    QAction *aAddCard, *aAddCardToSideboard, *aRemoveCard, *aIncrement, *aDecrement;
    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating,
        *aDeckAnalyticsDockFloating, *aDeckAnalyticsDockVisible, *aFilterDockVisible, *aFilterDockFloating;

    bool modified;
    QVBoxLayout *centralFrame;
    QVBoxLayout *searchAndDatabaseFrame;
    QHBoxLayout *searchLayout;
    QDockWidget *searchAndDatabaseDock;
    QDockWidget *cardInfoDock;
    QDockWidget *deckAnalyticsDock;
    QDockWidget *deckDock;
    QDockWidget *filterDock;
    QWidget *centralWidget;

public:
    explicit TabDeckEditorVisual(TabSupervisor *_tabSupervisor, QWidget *parent = nullptr);
    ~TabDeckEditorVisual() override;
    void retranslateUi() override;
    QString getTabText() const override;
    void setDeck(DeckLoader *_deckLoader);
    void setModified(bool _windowModified);
    bool confirmClose();
    void createDeckDock();
    void createCardInfoDock();
    void createDeckAnalyticsDock();
    void createFiltersDock();
    void createMenus();
    void createSearchAndDatabaseFrame();
    void createCentralFrame();

public slots:
    void setCurrentCardInfo(CardInfoPtr activeCard);
    void changeModelIndexAndCardInfo(CardInfoPtr activeCard);
    void processMainboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void processSideboardCardClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
    void processCardClickDatabaseDisplay(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *instance);
signals:
    void deckEditorClosing(TabDeckEditorVisual *tab);
};

#endif
