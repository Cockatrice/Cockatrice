#ifndef WINDOW_DECKEDITOR_H
#define WINDOW_DECKEDITOR_H

#include "tab.h"
#include <QAbstractItemModel>
#include <QLineEdit>
#include "decklist.h"

class CardDatabaseModel;
class CardDatabaseDisplayModel;
class DeckListModel;
class QTreeView;
class QTableView;
class CardInfoWidget;
class QTextEdit;
class DlgCardSearch;
class QLabel;

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

	void actEditSets();
	
	void actSearch();
	void actClearSearch();

	void actAddCard();
	void actAddCardToSideboard();
	void actRemoveCard();
	void actIncrement();
	void actDecrement();
        void actUpdatePrices();

        void finishedUpdatingPrices();
private:
	void addCardHelper(QString zoneName);
	void recursiveExpand(const QModelIndex &index);
	bool confirmClose();

	QString lastFileName;
	DeckList::FileFormat lastFileFormat;

	CardDatabaseModel *databaseModel;
	CardDatabaseDisplayModel *databaseDisplayModel;
	DeckListModel *deckModel;
	QTreeView *databaseView;
	QTreeView *deckView;
	CardInfoWidget *cardInfo;
	QLabel *searchLabel;
	SearchLineEdit *searchEdit;
	QLabel *nameLabel;
	QLineEdit *nameEdit;
	QLabel *commentsLabel;
	QTextEdit *commentsEdit;
	QLabel *hashLabel1;
	QLabel *hashLabel;
	DlgCardSearch *dlgCardSearch;

	QMenu *deckMenu, *dbMenu;
	QAction *aNewDeck, *aLoadDeck, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard, *aSaveDeckToClipboard, *aPrintDeck, *aClose;
	QAction *aEditSets, *aSearch, *aClearSearch;
        QAction *aAddCard, *aAddCardToSideboard, *aRemoveCard, *aIncrement, *aDecrement, *aUpdatePrices;
public:
	TabDeckEditor(TabSupervisor *_tabSupervisor, QWidget *parent = 0);
	~TabDeckEditor();
	void retranslateUi();
	QString getTabText() const;
	void setDeck(DeckList *_deck, const QString &_lastFileName = QString(), DeckList::FileFormat _lastFileFormat = DeckList::CockatriceFormat);
	void setWindowModified(bool _windowModified);
public slots:
	void closeRequest();
signals:
	void deckEditorClosing(TabDeckEditor *tab);
};

#endif
