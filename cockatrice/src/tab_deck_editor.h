#ifndef WINDOW_DECKEDITOR_H
#define WINDOW_DECKEDITOR_H

#include "tab.h"
#include <QAbstractItemModel>
#include <QLineEdit>
#include "keysignals.h"

class CardDatabaseModel;
class CardDatabaseDisplayModel;
class DeckListModel;
class QTreeView;
class QTableView;
class CardFrame;
class QTextEdit;
class QLabel;
class DeckLoader;
class Response;
class FilterTreeModel;
class CardInfo;

class SearchLineEdit : public QLineEdit {
    private:
        QTreeView *treeView;
    protected:
        void keyPressEvent(QKeyEvent *event);
    public:
        SearchLineEdit() : QLineEdit(), treeView(0) { }
        void setTreeView(QTreeView *_treeView) { treeView = _treeView; }
};

class TabDeckEditor : public Tab {
    Q_OBJECT
private slots:
    void updateName(const QString &name);
    void updateComments();
    void updateHash();
    void updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous);
    void updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous);
    void updateSearch(const QString &search);

    void actNewDeck();
    void actLoadDeck();
    bool actSaveDeck();
    bool actSaveDeckAs();
    void actLoadDeckFromClipboard();
    void actSaveDeckToClipboard();
    void actPrintDeck();
    void actAnalyzeDeck();

    void actEditSets();
    void actEditTokens();

    void actClearSearch();

    void actAddCard();
    void actAddCardToSideboard();
    void actRemoveCard();
    void actIncrement();
    void actDecrement();
    void actDecrementCard();
    void actDecrementCardFromSideboard();

    void actUpdatePrices();

    void finishedUpdatingPrices();
    void saveDeckRemoteFinished(const Response &r);
    void filterViewCustomContextMenu(const QPoint &point);
    void filterRemove(QAction *action);
    void setPriceTagFeatureEnabled(int enabled);
private:
    CardInfo *currentCardInfo() const;
    void addCardHelper(QString zoneName);
    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(QString zoneName);
    void recursiveExpand(const QModelIndex &index);
    bool confirmClose();

    CardDatabaseModel *databaseModel;
    CardDatabaseDisplayModel *databaseDisplayModel;
    DeckListModel *deckModel;
    QTreeView *databaseView;

    QTreeView *deckView;
    KeySignals deckViewKeySignals;
    CardFrame *cardInfo;
    QLabel *searchLabel;
    SearchLineEdit *searchEdit;
    KeySignals searchKeySignals;

    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *commentsLabel;
    QTextEdit *commentsEdit;
    QLabel *hashLabel1;
    QLabel *hashLabel;
    FilterTreeModel *filterModel;
    QTreeView *filterView;

    QMenu *deckMenu, *dbMenu;
    QAction *aNewDeck, *aLoadDeck, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard, *aSaveDeckToClipboard, *aPrintDeck, *aAnalyzeDeck, *aClose;
    QAction *aEditSets, *aEditTokens, *aClearSearch, *aCardTextOnly;
    QAction *aAddCard, *aAddCardToSideboard, *aRemoveCard, *aIncrement, *aDecrement, *aUpdatePrices;
    
    bool modified;
public:
    TabDeckEditor(TabSupervisor *_tabSupervisor, QWidget *parent = 0);
    ~TabDeckEditor();
    void retranslateUi();
    QString getTabText() const;
    void setDeck(DeckLoader *_deckLoader);
    void setModified(bool _windowModified);
public slots:
    void closeRequest();
signals:
    void deckEditorClosing(TabDeckEditor *tab);
};

#endif
