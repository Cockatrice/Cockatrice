#include "gamesmodel.h"
#include "servergame.h"

GamesModel::~GamesModel()
{
	cleanList();
}

int GamesModel::rowCount(const QModelIndex &parent) const
{
	return gameList.size();
}

int GamesModel::columnCount(const QModelIndex &parent) const
{
	return 5;
}

QVariant GamesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if ((index.row() >= gameList.size()) || (index.column() >= columnCount()))
		return QVariant();
	
	if (role != Qt::DisplayRole)
		return QVariant();
	
	ServerGame *g = gameList.at(index.row());
	switch (index.column()) {
		case 0: return g->getGameId();
		case 1: return g->getCreator();
		case 2: return g->getDescription();
		case 3: return QString(g->getHasPassword() ? "yes" : "no");
		case 4: return QString("%1/%2").arg(g->getPlayerCount()).arg(g->getMaxPlayers());
		default: return QVariant();
	}
}

QVariant GamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation != Qt::Horizontal)
		return QVariant();
	switch (section) {
		case 0: return QString("Game ID");
		case 1: return QString("Creator");
		case 2: return QString("Description");
		case 3: return QString("Password");
		case 4: return QString("Players");
		default: return QVariant();
	}
}

ServerGame *GamesModel::getGame(int row)
{
	if (row >= gameList.size())
		return 0;
	return gameList[row];
}

void GamesModel::setGameList(const QList<ServerGame *> &_gameList)
{
	cleanList();
	gameList = _gameList;
	reset();
}

void GamesModel::cleanList()
{
	QListIterator<ServerGame *> i(gameList);
	while (i.hasNext())
		delete i.next();
	gameList.clear();
}
