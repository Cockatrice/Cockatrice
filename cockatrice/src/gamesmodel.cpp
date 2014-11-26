#include "gamesmodel.h"
#include "pb/serverinfo_game.pb.h"
#include <QDebug>
#include <QStringList>
#include <QDateTime>

namespace {
    const unsigned SECS_PER_MIN  = 60;
    const unsigned SECS_PER_HOUR = 60 * 60;

    /**
     * Pretty print an integer number of seconds ago. Accurate to only one unit,
     * rounded; <5 minutes and >5 hours are displayed as such. As a special case, 
     * time between 60 and 90 minutes will display both the hours and minutes.
     *
     * For example...
     *   0-300 seconds will return "<5m ago"
     *   5-59 minutes will return "Xm ago"
     *   60-90 minutes will return "Xhr Ym ago"
     *   91-300 minutes will return "Xhr ago"
     *   300+ minutes will return "5+ hr ago"
     */
    QString prettyPrintSecsAgo(unsigned int secs) {
        if (secs < SECS_PER_MIN) {
            return QObject::tr("<1m ago");
        }
        if (secs < SECS_PER_MIN * 5) {
            return QObject::tr("<5m ago");
        }
        if (secs < SECS_PER_HOUR) {
            unsigned int mins = secs / SECS_PER_MIN;
            if (secs % SECS_PER_MIN >= 30)
              mins++;
            //: This will have a number prepended, like "10m ago"
            return QString::number(mins).append(QObject::tr("m ago"));
        }
        // Here, we want to display both the hours and minutes.
        //
        // There are two small "corner" cases which could be rectified with
        // some more knotty iffy-elsey code:
        //   Between 1:00:00 and 1:00:29 will display "1hr 0m ago"
        //   Between 1:29:30 and 1:29:59 will display "1hr 31m ago"
        //
        // Personally, I prefer to keep the code cleaner, and allow these.
        if (secs < SECS_PER_MIN * 90) {
          unsigned int mins = secs / SECS_PER_MIN - 60;
          if (secs % SECS_PER_MIN >= 30)
            mins++;
          return QObject::tr("1hr ")
                   .append(QString::number(mins))
                   //: This will have a number prepended, like "5m ago"
                   .append(QObject::tr("m ago"));
        }
        if (secs < SECS_PER_HOUR * 5) {
            unsigned int hours = secs / SECS_PER_HOUR;
            if (secs % SECS_PER_HOUR >= SECS_PER_MIN * 30)
              hours++;
            //: This will have a number prepended, like "2h ago"
            return QString::number(hours).append(QObject::tr("hr ago"));
        }
        return QObject::tr("5+ hrs ago");
    }
}

GamesModel::GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent)
    : QAbstractTableModel(parent), rooms(_rooms), gameTypes(_gameTypes)
{
}

QVariant GamesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::UserRole)
        return index.row();
    if (role != Qt::DisplayRole && role != SORT_ROLE)
        return QVariant();
    if ((index.row() >= gameList.size()) || (index.column() >= columnCount()))
        return QVariant();

    const ServerInfo_Game &g = gameList[index.row()];
    switch (index.column()) {
        case 0: return rooms.value(g.room_id());
        case 1: {
            QDateTime then;
            then.setTime_t(g.start_time());
            unsigned int secs = then.secsTo(QDateTime::currentDateTime());
 
            switch (role) {
                case Qt::DisplayRole: return prettyPrintSecsAgo(secs);
                case SORT_ROLE: return QVariant(secs);
                default: {
                    qDebug() << "Returning data for col 1 of games model when role != display, role != sort";
                    return QVariant(); // Shouldn't ever be reached.
                }
            }
        }
        case 2: return QString::fromStdString(g.description());
        case 3: return QString::fromStdString(g.creator_info().name());
        case 4: {
            QStringList result;
            GameTypeMap gameTypeMap = gameTypes.value(g.room_id());
            for (int i = g.game_types_size() - 1; i >= 0; --i)
                result.append(gameTypeMap.value(g.game_types(i)));
            return result.join(", ");
        }
        case 5: return g.with_password() ? ((g.spectators_need_password() || !g.spectators_allowed()) ? tr("yes") : tr("yes, free for spectators")) : tr("no");
        case 6: {
            QStringList result;
            if (g.only_buddies())
                result.append(tr("buddies only"));
            if (g.only_registered())
                result.append(tr("reg. users only"));
            return result.join(", ");
        }
        case 7: return QString("%1/%2").arg(g.player_count()).arg(g.max_players());
        case 8: return g.spectators_allowed() ? QVariant(g.spectators_count()) : QVariant(tr("not allowed"));
        default: return QVariant();
    }
}

QVariant GamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
        return QVariant();
    switch (section) {
        case 0: return tr("Room");
        case 1: return tr("Game Created");
        case 2: return tr("Description");
        case 3: return tr("Creator");
        case 4: return tr("Game Type");
        case 5: return tr("Password");
        case 6: return tr("Restrictions");
        case 7: return tr("Players");
        case 8: return tr("Spectators");
        default: return QVariant();
    }
}

const ServerInfo_Game &GamesModel::getGame(int row)
{
    Q_ASSERT(row < gameList.size());
    return gameList[row];
}

void GamesModel::updateGameList(const ServerInfo_Game &game)
{
    for (int i = 0; i < gameList.size(); i++) {
        if (gameList[i].game_id() == game.game_id()) {
            if (game.closed()) {
                beginRemoveRows(QModelIndex(), i, i);
                gameList.removeAt(i);
                endRemoveRows();
            } else {
                gameList[i].MergeFrom(game);
                emit dataChanged(index(i, 0), index(i, NUM_COLS-1));
            }
            return;
        }
    }
    if (game.player_count() <= 0)
        return;
    beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
    gameList.append(game);
    endInsertRows();
}

GamesProxyModel::GamesProxyModel(QObject *parent, ServerInfo_User *_ownUser)
    : QSortFilterProxyModel(parent),
          ownUser(_ownUser),
          unavailableGamesVisible(false),
          passwordProtectedGamesVisible(false),
          maxPlayersFilterMin(-1),
          maxPlayersFilterMax(-1)
{
    setSortRole(GamesModel::SORT_ROLE);
    setDynamicSortFilter(true);
}

void GamesProxyModel::setUnavailableGamesVisible(bool _unavailableGamesVisible)
{
    unavailableGamesVisible = _unavailableGamesVisible;
    invalidateFilter();
}

void GamesProxyModel::setPasswordProtectedGamesVisible(bool _passwordProtectedGamesVisible)
{
    passwordProtectedGamesVisible = _passwordProtectedGamesVisible;
    invalidateFilter();
}

void GamesProxyModel::setGameNameFilter(const QString &_gameNameFilter)
{
    gameNameFilter = _gameNameFilter;
    invalidateFilter();
}

void GamesProxyModel::setCreatorNameFilter(const QString &_creatorNameFilter)
{
    creatorNameFilter = _creatorNameFilter;
    invalidateFilter();
}

void GamesProxyModel::setGameTypeFilter(const QSet<int> &_gameTypeFilter)
{
    gameTypeFilter = _gameTypeFilter;
    invalidateFilter();
}

void GamesProxyModel::setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax)
{
    maxPlayersFilterMin = _maxPlayersFilterMin;
    maxPlayersFilterMax = _maxPlayersFilterMax;
    invalidateFilter();
}

void GamesProxyModel::resetFilterParameters()
{
    unavailableGamesVisible = false;
    passwordProtectedGamesVisible = false;
    gameNameFilter = QString();
    creatorNameFilter = QString();
    gameTypeFilter.clear();
    maxPlayersFilterMin = -1;
    maxPlayersFilterMax = -1;

    invalidateFilter();
}

bool GamesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    GamesModel *model = qobject_cast<GamesModel *>(sourceModel());
    if (!model)
        return false;

    const ServerInfo_Game &game = model->getGame(sourceRow);
    if (!unavailableGamesVisible) {
        if (game.player_count() == game.max_players())
            return false;
        if (game.started())
            return false;
        if (!(ownUser->user_level() & ServerInfo_User::IsRegistered))
            if (game.only_registered())
                return false;
    }
    if (!passwordProtectedGamesVisible && game.with_password())
        return false;
    if (!gameNameFilter.isEmpty())
        if (!QString::fromStdString(game.description()).contains(gameNameFilter, Qt::CaseInsensitive))
            return false;
    if (!creatorNameFilter.isEmpty())
        if (!QString::fromStdString(game.creator_info().name()).contains(creatorNameFilter, Qt::CaseInsensitive))
            return false;

    QSet<int> gameTypes;
    for (int i = 0; i < game.game_types_size(); ++i)
        gameTypes.insert(game.game_types(i));
    if (!gameTypeFilter.isEmpty() && gameTypes.intersect(gameTypeFilter).isEmpty())
        return false;

    if ((maxPlayersFilterMin != -1) && ((int)game.max_players() < maxPlayersFilterMin))
        return false;
    if ((maxPlayersFilterMax != -1) && ((int)game.max_players() > maxPlayersFilterMax))
        return false;

    return true;
}
