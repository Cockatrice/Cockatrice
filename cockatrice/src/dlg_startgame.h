#ifndef DLG_STARTGAME_H
#define DLG_STARTGAME_H

#include <QDialog>

class QTreeView;
class QPushButton;
class CardDatabase;
class DeckListModel;

class DlgStartGame: public QDialog {
	Q_OBJECT
public:
	DlgStartGame(QWidget *parent = 0);
	QStringList getDeckList() const;
signals:
	void newDeckLoaded(const QStringList &cards);
private slots:
	void actLoad();
private:
	QTreeView *deckView;
	DeckListModel *deckModel;

	QPushButton *loadButton, *okButton;
};

#endif
