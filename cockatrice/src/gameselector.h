#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include <QWidget>
#include "gamesmodel.h"
#include "servergame.h"
#include "client.h"

class QPushButton;
class QTreeView;

class GameSelector : public QWidget {
	Q_OBJECT
public:
	GameSelector(Client *_client, QWidget *parent = 0);
	void enableGameList();
	void disableGameList();
	void retranslateUi();
private slots:
	void actCreate();
	void actRefresh();
	void actJoin();
	void checkResponse(ServerResponse response);
private:
	Client *client;

	QTreeView *gameListView;
	GamesModel *gameListModel;
	QPushButton *createButton, *joinButton, *spectateButton;
};

#endif

