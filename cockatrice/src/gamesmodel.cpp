#include "gamesmodel.h"
#include "client.h"

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
	
	const ServerGame &g = gameList[index.row()];
	switch (index.column()) {
		case 0: return g.getDescription();
		case 1: return g.getCreator();
		case 2: return g.getHasPassword() ? tr("yes") : tr("no");
		case 3: return QString("%1/%2").arg(g.getPlayerCount()).arg(g.getMaxPlayers());
		case 4: return g.getSpectatorsAllowed() ? QVariant(g.getSpectatorsCount()) : QVariant(tr("not allowed"));
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

const ServerGame &GamesModel::getGame(int row)
{
	Q_ASSERT(row < gameList.size());
	return gameList[row];
}

void GamesModel::updateGameList(const ServerGame &game)
{
	for (int i = 0; i < gameList.size(); i++)
		if (gameList[i].getGameId() == game.getGameId()) {
			if (game.getPlayerCount() == 0) {
				beginRemoveRows(QModelIndex(), i, i);
				gameList.removeAt(i);
				endRemoveRows();
			} else {
				gameList[i] = game;
				emit dataChanged(index(i, 0), index(i, 4));
			}
			return;
		}
	if (game.getPlayerCount() == 0)
		return;
	beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
	gameList << game;
	endInsertRows();
}

void GamesModel::cleanList()
{
	if (gameList.isEmpty())
		return;

	beginRemoveRows(QModelIndex(), 0, gameList.size() - 1);
	gameList.clear();
	endRemoveRows();
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
	
	const ServerGame &game = model->getGame(sourceRow);
	if (game.getPlayerCount() == game.getMaxPlayers())
		return false;
	
	return true;
}
