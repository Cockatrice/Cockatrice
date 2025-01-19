#ifndef TAB_DECK_EDITOR_VISUAL_DATABASE_SEARCH_TAB_H
#define TAB_DECK_EDITOR_VISUAL_DATABASE_SEARCH_TAB_H
#include "../../../client/game_logic/key_signals.h"
#include "../../../deck/deck_list_model.h"
#include "../../../game/cards/card_database.h"
#include "../../../game/cards/card_database_model.h"
#include "../../ui/widgets/cards/card_info_frame_widget.h"
#include "../../ui/widgets/deck_analytics/deck_analytics_widget.h"
#include "../../ui/widgets/visual_deck_editor/visual_deck_editor_widget.h"
#include "../tab.h"
#include "../tab_deck_editor.h"

#include <QTreeView>

class TabDeckEditorVisualDatabaseSearchTab : public Tab
{
    Q_OBJECT
private slots:
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);
    void updateSearch(const QString &search);
    void databaseCustomMenu(QPoint point);

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

    void filterViewCustomContextMenu(const QPoint &point);
    void filterRemove(QAction *action);

    void saveDbHeaderState();
    void setSaveStatus(bool newStatus);
    void showSearchSyntaxHelp();

private:
    QWidget *parent;
    CardInfoPtr currentCardInfo() const;
    void addCardHelper(QString zoneName);
    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(QString zoneName);
    void recursiveExpand(const QModelIndex &index);

    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    DeckListModel *deckModel;
    QTreeView *databaseView;

    VisualDeckEditorWidget *visualDeckView;
    CardInfoFrameWidget *cardInfo;
    SearchLineEdit *searchEdit;
    KeySignals searchKeySignals;

    FilterTreeModel *filterModel;
    QTreeView *filterView;
    KeySignals filterViewKeySignals;
    QWidget *filterBox;

    QMenu *deckMenu, *viewMenu, *cardInfoDockMenu, *filterDockMenu;
    QAction *aClose;
    QAction *aClearFilterAll, *aClearFilterOne;
    QAction *aAddCard, *aAddCardToSideboard, *aRemoveCard, *aIncrement, *aDecrement;
    QAction *aResetLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aDeckDockVisible, *aDeckDockFloating,
        *aDeckAnalyticsDockFloating, *aDeckAnalyticsDockVisible, *aFilterDockVisible, *aFilterDockFloating;

    bool modified;
    QVBoxLayout *layout;
    QVBoxLayout *searchAndDatabaseFrame;
    QHBoxLayout *searchLayout;
    QDockWidget *searchAndDatabaseDock;
    QDockWidget *cardInfoDock;
    QDockWidget *deckDock;
    QDockWidget *filterDock;
    QWidget *centralWidget;

public:
    explicit TabDeckEditorVisualDatabaseSearchTab(TabSupervisor *_tabSupervisor, QWidget *_parent);
    ~TabDeckEditorVisualDatabaseSearchTab() override;
    void retranslateUi() override;
    void setDeck(DeckLoader *_deckLoader);
    void setModified(bool _windowModified);
    bool confirmClose();

signals:
    void deckEditorClosing(TabDeckEditorVisualDatabaseSearchTab *tab);
};

#endif // TAB_DECK_EDITOR_VISUAL_DATABASE_SEARCH_TAB_H
