#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QSet>
#include "gametypemap.h"

class ServerInfo_Game;
class ServerInfo_User;

class GamesModel : public QAbstractTableModel {
	Q_OBJECT
private:
	QList<ServerInfo_Game> gameList;
	QMap<int, QString> rooms;
	QMap<int, GameTypeMap> gameTypes;
public:
	GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent = 0);
	~GamesModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const { return parent.isValid() ? 0 : gameList.size(); }
	int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return 8; }
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	
	const ServerInfo_Game &getGame(int row);
	void updateGameList(const ServerInfo_Game &game);
};

class GamesProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
private:
	ServerInfo_User *ownUser;
	bool unavailableGamesVisible;
	QString gameNameFilter, creatorNameFilter;
	QSet<int> gameTypeFilter;
	int maxPlayersFilterMin, maxPlayersFilterMax;
public:
	GamesProxyModel(QObject *parent = 0, ServerInfo_User *_ownUser = 0);
	
	void setUnavailableGamesVisible(bool _unavailableGamesVisible);
	void setGameNameFilter(const QString &_gameNameFilter);
	void setCreatorNameFilter(const QString &_creatorNameFilter);
	void setGameTypeFilter(const QSet<int> &_gameTypeFilter);
	void setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax);
	void resetFilterParameters();
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
