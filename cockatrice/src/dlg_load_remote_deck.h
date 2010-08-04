#ifndef DLG_STARTGAME_H
#define DLG_STARTGAME_H

#include <QDialog>

class RemoteDeckList_TreeWidget;
class QModelIndex;
class AbstractClient;
class QPushButton;

class DlgLoadRemoteDeck: public QDialog {
	Q_OBJECT
private:
	AbstractClient *client;
	RemoteDeckList_TreeWidget *dirView;
	QPushButton *okButton, *cancelButton;
private slots:
	void currentItemChanged(const QModelIndex &current, const QModelIndex &previous);
public:
	DlgLoadRemoteDeck(AbstractClient *_client, QWidget *parent = 0);
	int getDeckId() const;
};

#endif
