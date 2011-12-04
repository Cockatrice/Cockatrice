#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QList>
#include "gametypemap.h"
#include "protocol_datastructures.h"

class ServerInfo_Game;

class GamesModel : public QAbstractTableModel {
	Q_OBJECT
private:
	QList<ServerInfo_Game *> gameList;
	QMap<int, QString> rooms;
	QMap<int, GameTypeMap> gameTypes;
public:
	GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent = 0);
	~GamesModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const { return parent.isValid() ? 0 : gameList.size(); }
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return 8; }
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	ServerInfo_Game *getGame(int row);
	void updateGameList(ServerInfo_Game *game);
};

class GamesProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
private:
	ServerInfo_User *ownUser;
	bool unjoinableGamesVisible;
public:
	GamesProxyModel(QObject *parent = 0, ServerInfo_User *_ownUser = 0);
	void setUnjoinableGamesVisible(bool _unjoinableGamesVisible);
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
