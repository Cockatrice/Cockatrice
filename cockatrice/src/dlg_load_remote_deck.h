#ifndef DLG_STARTGAME_H
#define DLG_STARTGAME_H

#include <QDialog>

class RemoteDeckList_TreeWidget;
class QTreeWidgetItem;
class Client;
class QPushButton;

class DlgLoadRemoteDeck: public QDialog {
	Q_OBJECT
private:
	Client *client;
	RemoteDeckList_TreeWidget *dirView;
	QPushButton *okButton, *cancelButton;
private slots:
	void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
public:
	DlgLoadRemoteDeck(Client *_client, QWidget *parent = 0);
	int getDeckId() const;
};

#endif
