#include "gamesmodel.h"
#include "protocol_datastructures.h"

GamesModel::~GamesModel()
{
	if (!gameList.isEmpty()) {
		beginRemoveRows(QModelIndex(), 0, gameList.size() - 1);
		for (int i = 0; i < gameList.size(); ++i)
			delete gameList[i];
		gameList.clear();
		endRemoveRows();
	}
}

QVariant GamesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::UserRole)
		return index.row();
	if (role != Qt::DisplayRole)
		return QVariant();
	if ((index.row() >= gameList.size()) || (index.column() >= columnCount()))
		return QVariant();
	
	ServerInfo_Game *g = gameList[index.row()];
	switch (index.column()) {
		case 0: return g->getDescription();
		case 1: return g->getCreatorInfo()->getName();
		case 2: return g->getHasPassword() ? (g->getSpectatorsNeedPassword() ? tr("yes") : tr("yes, free for spectators")) : tr("no");
		case 3: return QString("%1/%2").arg(g->getPlayerCount()).arg(g->getMaxPlayers());
		case 4: return g->getSpectatorsAllowed() ? QVariant(g->getSpectatorCount()) : QVariant(tr("not allowed"));
		default: return QVariant();
	}
}

QVariant GamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
		return QVariant();
	switch (section) {
		case 0: return tr("Description");
		case 1: return tr("Creator");
		case 2: return tr("Password");
		case 3: return tr("Players");
		case 4: return tr("Spectators");
		default: return QVariant();
	}
}

ServerInfo_Game *GamesModel::getGame(int row)
{
	Q_ASSERT(row < gameList.size());
	return gameList[row];
}

void GamesModel::updateGameList(ServerInfo_Game *_game)
{
	ServerInfo_Game *game = new ServerInfo_Game(_game->getGameId(), _game->getDescription(), _game->getHasPassword(), _game->getPlayerCount(), _game->getMaxPlayers(), new ServerInfo_User(_game->getCreatorInfo()), _game->getSpectatorsAllowed(), _game->getSpectatorsNeedPassword(), _game->getSpectatorCount());
	for (int i = 0; i < gameList.size(); i++)
		if (gameList[i]->getGameId() == game->getGameId()) {
			if (game->getPlayerCount() == 0) {
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
	if (game->getPlayerCount() == 0)
		return;
	beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
	gameList.append(game);
	endInsertRows();
}

GamesProxyModel::GamesProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent), fullGamesVisible(false)
{
	setDynamicSortFilter(true);
}

void GamesProxyModel::setFullGamesVisible(bool _fullGamesVisible)
{
	fullGamesVisible = _fullGamesVisible;
	invalidateFilter();
}

bool GamesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
	if (fullGamesVisible)
		return true;
	
	GamesModel *model = qobject_cast<GamesModel *>(sourceModel());
	if (!model)
		return false;
	
	ServerInfo_Game *game = model->getGame(sourceRow);
	if (game->getPlayerCount() == game->getMaxPlayers())
		return false;
	
	return true;
}
