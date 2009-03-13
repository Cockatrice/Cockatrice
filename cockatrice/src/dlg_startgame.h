#ifndef DLG_STARTGAME_H
#define DLG_STARTGAME_H

#include <QDialog>
#include "decklistmodel.h"

class QTreeView;
class QPushButton;
class CardDatabase;

class DlgStartGame: public QDialog {
	Q_OBJECT
public:
	DlgStartGame(CardDatabase *_db, QWidget *parent = 0);
	QStringList getDeckList() const;
signals:
	void newDeckLoaded(const QStringList &cards);
private slots:
	void actLoad();
private:
	CardDatabase *db;
	QTreeView *tableView;
	DeckListModel *tableModel;

	QPushButton *loadButton, *okButton;
};

#endif
