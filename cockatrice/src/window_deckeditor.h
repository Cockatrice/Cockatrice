#ifndef WINDOW_DECKEDITOR_H
#define WINDOW_DECKEDITOR_H

#include <QMainWindow>
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

class SearchLineEdit : public QLineEdit {
	private:
		QTreeView *treeView;
	protected:
		void keyPressEvent(QKeyEvent *event);
	public:
		SearchLineEdit() : QLineEdit(), treeView(0) { }
		void setTreeView(QTreeView *_treeView) { treeView = _treeView; }
};

class WndDeckEditor : public QMainWindow {
	Q_OBJECT
private slots:
	void updateName(const QString &name);
	void updateComments();
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
private:
	void addCardHelper(const QString &zoneName);
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
	SearchLineEdit *searchEdit;
	QLineEdit *nameEdit;
	QTextEdit *commentsEdit;
	DlgCardSearch *dlgCardSearch;

	QMenu *deckMenu, *dbMenu;
	QAction *aNewDeck, *aLoadDeck, *aSaveDeck, *aSaveDeckAs, *aLoadDeckFromClipboard, *aSaveDeckToClipboard, *aPrintDeck, *aClose;
	QAction *aEditSets, *aSearch, *aClearSearch;
	QAction *aAddCard, *aAddCardToSideboard, *aRemoveCard, *aIncrement, *aDecrement;
public:
	WndDeckEditor(QWidget *parent = 0);
	~WndDeckEditor();
	void setDeck(DeckList *_deck, const QString &_lastFileName = QString(), DeckList::FileFormat _lastFileFormat = DeckList::CockatriceFormat);
protected:
	void closeEvent(QCloseEvent *event);
};

#endif
