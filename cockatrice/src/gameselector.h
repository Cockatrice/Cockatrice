#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include <QWidget>
#include "client.h"

class QPushButton;
class QCheckBox;
class QTreeView;
class GamesModel;
class GamesProxyModel;

class GameSelector : public QWidget {
	Q_OBJECT
public:
	GameSelector(Client *_client, QWidget *parent = 0);
	void enableGameList();
	void disableGameList();
	void retranslateUi();
private slots:
	void showFullGamesChanged(int state);
	void actCreate();
	void actRefresh();
	void actJoin();
	void checkResponse(ServerResponse response);
private:
	Client *client;

	QTreeView *gameListView;
	GamesModel *gameListModel;
	GamesProxyModel *gameListProxyModel;
	QPushButton *createButton, *joinButton, *spectateButton;
	QCheckBox *showFullGamesCheckBox;
};

#endif

