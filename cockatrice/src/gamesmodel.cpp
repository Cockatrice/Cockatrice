#include "gamesmodel.h"
#include "servergame.h"

GamesModel::~GamesModel()
{
	cleanList();
}

QVariant GamesModel::data(const QModelIndex &index, int role) const
{
	if ((role != Qt::DisplayRole) || !index.isValid())
		return QVariant();
	if ((index.row() >= gameList.size()) || (index.column() >= columnCount()))
		return QVariant();
	
	ServerGame *g = gameList.at(index.row());
	switch (index.column()) {
		case 0: return g->getGameId();
		case 1: return g->getCreator();
		case 2: return g->getDescription();
		case 3: return QString(g->getHasPassword() ? tr("yes") : tr("no"));
		case 4: return QString("%1/%2").arg(g->getPlayerCount()).arg(g->getMaxPlayers());
		default: return QVariant();
	}
}

QVariant GamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
		return QVariant();
	switch (section) {
		case 0: return tr("Game ID");
		case 1: return tr("Creator");
		case 2: return tr("Description");
		case 3: return tr("Password");
		case 4: return tr("Players");
		default: return QVariant();
	}
}

ServerGame *GamesModel::getGame(int row)
{
	if (row >= gameList.size())
		return 0;
	return gameList[row];
}

void GamesModel::updateGameList(ServerGame *game)
{
	for (int i = 0; i < gameList.size(); i++)
		if (gameList[i]->getGameId() == game->getGameId()) {
			if ((game->getPlayerCount() == 0) || (game->getPlayerCount() == game->getMaxPlayers())) {
				beginRemoveRows(QModelIndex(), i, i);
				delete gameList.takeAt(i);
				endRemoveRows();
			} else {
				delete gameList[i];
				gameList[i] = game;
				emit dataChanged(index(i, 0), index(i, 4));
			}
			return;
		}
	if ((game->getPlayerCount() == 0) || (game->getPlayerCount() == game->getMaxPlayers()))
		return;
	beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
	gameList << game;
	endInsertRows();
}

void GamesModel::cleanList()
{
	QListIterator<ServerGame *> i(gameList);
	while (i.hasNext())
		delete i.next();
	gameList.clear();
}
