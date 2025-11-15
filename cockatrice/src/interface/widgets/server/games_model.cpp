#include "games_model.h"

#include "../../../client/settings/cache_settings.h"
#include "../interface/pixel_map_generator.h"
#include "../interface/widgets/tabs/tab_account.h"
#include "user/user_list_manager.h"
#include "user/user_list_widget.h"

#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QStringList>
#include <QTime>
#include <QTimeZone>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

enum GameListColumn
{
    ROOM,
    CREATED,
    DESCRIPTION,
    CREATOR,
    GAME_TYPE,
    RESTRICTIONS,
    PLAYERS,
    SPECTATORS
};

const int DEFAULT_MAX_PLAYERS_MIN = 1;
const int DEFAULT_MAX_PLAYERS_MAX = 99;
constexpr auto DEFAULT_MAX_GAME_AGE = QTime();

const QString GamesModel::getGameCreatedString(const int secs)
{
    static const QTime zeroTime{0, 0};
    static const int halfHourSecs = zeroTime.secsTo(QTime(1, 0)) / 2;
    static const int halfMinSecs = zeroTime.secsTo(QTime(0, 1)) / 2;
    static const int wrapSeconds = zeroTime.secsTo(zeroTime.addSecs(-halfHourSecs)); // round up

    if (secs >= wrapSeconds) { // QTime wraps after a day
        return tr(">1 day");
    }

    QTime total = zeroTime.addSecs(secs);
    QTime totalRounded = total.addSecs(halfMinSecs); // round up
    QString form;
    int amount;
    if (totalRounded.hour()) {
        amount = total.addSecs(halfHourSecs).hour(); // round up separately
        form = tr("%1%2 hr", "short age in hours", amount);
    } else if (total.minute() < 2) { // games are new during their first minute
        return tr("new");
    } else {
        amount = totalRounded.minute();
        form = tr("%1%2 min", "short age in minutes", amount);
    }

    for (int aggregate : {40, 20, 10, 5}) { // floor to values in this list
        if (amount >= aggregate) {
            return form.arg(">").arg(aggregate);
        }
    }
    return form.arg("").arg(amount);
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
    if (role != Qt::DisplayRole && role != SORT_ROLE && role != Qt::DecorationRole && role != Qt::TextAlignmentRole)
        return QVariant();
    if ((index.row() >= gameList.size()) || (index.column() >= columnCount()))
        return QVariant();

    const ServerInfo_Game &gameentry = gameList[index.row()];
    switch (index.column()) {
        case ROOM:
            return rooms.value(gameentry.room_id());
        case CREATED: {
            switch (role) {
                case Qt::DisplayRole: {
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                    auto then = QDateTime::fromSecsSinceEpoch(gameentry.start_time(), QTimeZone::UTC);
#else
                    auto then = QDateTime::fromSecsSinceEpoch(gameentry.start_time(), Qt::UTC);
#endif
                    int secs = then.secsTo(QDateTime::currentDateTimeUtc());
                    return getGameCreatedString(secs);
                }
                case SORT_ROLE:
                    return QVariant(-static_cast<qint64>(gameentry.start_time()));
                case Qt::TextAlignmentRole:
                    return Qt::AlignCenter;
                default:
                    return QVariant();
            }
        }
        case DESCRIPTION:
            switch (role) {
                case SORT_ROLE:
                case Qt::DisplayRole:
                    return QString::fromStdString(gameentry.description());
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                default:
                    return QVariant();
            }
        case CREATOR: {
            switch (role) {
                case SORT_ROLE:
                case Qt::DisplayRole:
                    return QString::fromStdString(gameentry.creator_info().name());
                case Qt::DecorationRole: {
                    return UserLevelPixmapGenerator::generateIcon(
                        13, UserLevelFlags(gameentry.creator_info().user_level()),
                        gameentry.creator_info().pawn_colors(), false,
                        QString::fromStdString(gameentry.creator_info().privlevel()));
                }
                default:
                    return QVariant();
            }
        }
        case GAME_TYPE:
            switch (role) {
                case SORT_ROLE:
                case Qt::DisplayRole: {
                    QStringList result;
                    GameTypeMap gameTypeMap = gameTypes.value(gameentry.room_id());
                    for (int i = gameentry.game_types_size() - 1; i >= 0; --i)
                        result.append(gameTypeMap.value(gameentry.game_types(i)));
                    return result.join(", ");
                }
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                default:
                    return QVariant();
            }
        case RESTRICTIONS:
            switch (role) {
                case SORT_ROLE:
                case Qt::DisplayRole: {
                    QStringList result;
                    if (gameentry.with_password())
                        result.append(tr("password"));
                    if (gameentry.only_buddies())
                        result.append(tr("buddies only"));
                    if (gameentry.only_registered())
                        result.append(tr("reg. users only"));
                    if (gameentry.share_decklists_on_load())
                        result.append(tr("open decklists"));
                    return result.join(", ");
                }
                case Qt::DecorationRole: {
                    return gameentry.with_password() ? QIcon(LockPixmapGenerator::generatePixmap(13)) : QVariant();
                    case Qt::TextAlignmentRole:
                        return Qt::AlignLeft;
                    default:
                        return QVariant();
                }
            }
        case PLAYERS:
            switch (role) {
                case SORT_ROLE:
                case Qt::DisplayRole:
                    return QString("%1/%2").arg(gameentry.player_count()).arg(gameentry.max_players());
                case Qt::TextAlignmentRole:
                    return Qt::AlignCenter;
                default:
                    return QVariant();
            }

        case SPECTATORS:
            switch (role) {
                case SORT_ROLE:
                case Qt::DisplayRole: {
                    if (gameentry.spectators_allowed()) {
                        QString result;
                        result.append(QString::number(gameentry.spectators_count()));

                        if (gameentry.spectators_can_chat() && gameentry.spectators_omniscient()) {
                            result.append(" (")
                                .append(tr("can chat"))
                                .append(" & ")
                                .append(tr("see hands"))
                                .append(")");
                        } else if (gameentry.spectators_can_chat()) {
                            result.append(" (").append(tr("can chat")).append(")");
                        } else if (gameentry.spectators_omniscient()) {
                            result.append(" (").append(tr("can see hands")).append(")");
                        }

                        return result;
                    }
                    return QVariant(tr("not allowed"));
                }
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                default:
                    return QVariant();
            }
        default:
            return QVariant();
    }
}

QVariant GamesModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if ((role != Qt::DisplayRole) && (role != Qt::TextAlignmentRole))
        return QVariant();
    switch (section) {
        case ROOM:
            return tr("Room");
        case CREATED: {
            switch (role) {
                case Qt::DisplayRole:
                    return tr("Age");
                case Qt::TextAlignmentRole:
                    return Qt::AlignCenter;
                default:
                    return QVariant();
            }
        }
        case DESCRIPTION:
            return tr("Description");
        case CREATOR:
            return tr("Creator");
        case GAME_TYPE:
            return tr("Type");
        case RESTRICTIONS:
            return tr("Restrictions");
        case PLAYERS: {
            switch (role) {
                case Qt::DisplayRole:
                    return tr("Players");
                case Qt::TextAlignmentRole:
                    return Qt::AlignCenter;
                default:
                    return QVariant();
            }
        }
        case SPECTATORS:
            return tr("Spectators");
        default:
            return QVariant();
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
                emit dataChanged(index(i, 0), index(i, NUM_COLS - 1));
            }
            return;
        }
    }
    beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
    gameList.append(game);
    endInsertRows();
}

GamesProxyModel::GamesProxyModel(QObject *parent, const UserListProxy *_userListProxy)
    : QSortFilterProxyModel(parent), userListProxy(_userListProxy)
{
    resetFilterParameters();
    setSortRole(GamesModel::SORT_ROLE);
    setDynamicSortFilter(true);
}

void GamesProxyModel::setGameFilters(bool _hideBuddiesOnlyGames,
                                     bool _hideIgnoredUserGames,
                                     bool _hideFullGames,
                                     bool _hideGamesThatStarted,
                                     bool _hidePasswordProtectedGames,
                                     bool _hideNotBuddyCreatedGames,
                                     bool _hideOpenDecklistGames,
                                     const QString &_gameNameFilter,
                                     const QStringList &_creatorNameFilters,
                                     const QSet<int> &_gameTypeFilter,
                                     int _maxPlayersFilterMin,
                                     int _maxPlayersFilterMax,
                                     const QTime &_maxGameAge,
                                     bool _showOnlyIfSpectatorsCanWatch,
                                     bool _showSpectatorPasswordProtected,
                                     bool _showOnlyIfSpectatorsCanChat,
                                     bool _showOnlyIfSpectatorsCanSeeHands)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))
    beginFilterChange();
#endif
    hideBuddiesOnlyGames = _hideBuddiesOnlyGames;
    hideIgnoredUserGames = _hideIgnoredUserGames;
    hideFullGames = _hideFullGames;
    hideGamesThatStarted = _hideGamesThatStarted;
    hidePasswordProtectedGames = _hidePasswordProtectedGames;
    hideNotBuddyCreatedGames = _hideNotBuddyCreatedGames;
    hideOpenDecklistGames = _hideOpenDecklistGames;
    gameNameFilter = _gameNameFilter;
    creatorNameFilters = _creatorNameFilters;
    gameTypeFilter = _gameTypeFilter;
    maxPlayersFilterMin = _maxPlayersFilterMin;
    maxPlayersFilterMax = _maxPlayersFilterMax;
    maxGameAge = _maxGameAge;
    showOnlyIfSpectatorsCanWatch = _showOnlyIfSpectatorsCanWatch;
    showSpectatorPasswordProtected = _showSpectatorPasswordProtected;
    showOnlyIfSpectatorsCanChat = _showOnlyIfSpectatorsCanChat;
    showOnlyIfSpectatorsCanSeeHands = _showOnlyIfSpectatorsCanSeeHands;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
}

int GamesProxyModel::getNumFilteredGames() const
{
    auto *model = qobject_cast<GamesModel *>(sourceModel());
    if (!model)
        return 0;

    int numFilteredGames = 0;
    for (int row = 0; row < model->rowCount(); ++row) {
        if (!filterAcceptsRow(row)) {
            ++numFilteredGames;
        }
    }
    return numFilteredGames;
}

void GamesProxyModel::resetFilterParameters()
{
    setGameFilters(false, false, false, false, false, false, false, QString(), QStringList(), {},
                   DEFAULT_MAX_PLAYERS_MIN, DEFAULT_MAX_PLAYERS_MAX, DEFAULT_MAX_GAME_AGE, false, false, false, false);
}

bool GamesProxyModel::areFilterParametersSetToDefaults() const
{
    return !hideFullGames && !hideGamesThatStarted && !hidePasswordProtectedGames && !hideBuddiesOnlyGames &&
           !hideOpenDecklistGames && !hideIgnoredUserGames && !hideNotBuddyCreatedGames && gameNameFilter.isEmpty() &&
           creatorNameFilters.isEmpty() && gameTypeFilter.isEmpty() && maxPlayersFilterMin == DEFAULT_MAX_PLAYERS_MIN &&
           maxPlayersFilterMax == DEFAULT_MAX_PLAYERS_MAX && maxGameAge == DEFAULT_MAX_GAME_AGE &&
           !showOnlyIfSpectatorsCanWatch && !showSpectatorPasswordProtected && !showOnlyIfSpectatorsCanChat &&
           !showOnlyIfSpectatorsCanSeeHands;
}

void GamesProxyModel::loadFilterParameters(const QMap<int, QString> &allGameTypes)
{
    GameFiltersSettings &gameFilters = SettingsCache::instance().gameFilters();

    QSet<int> newGameTypeFilter;
    QMapIterator<int, QString> gameTypesIterator(allGameTypes);
    while (gameTypesIterator.hasNext()) {
        gameTypesIterator.next();
        if (gameFilters.isGameTypeEnabled(gameTypesIterator.value())) {
            newGameTypeFilter.insert(gameTypesIterator.key());
        }
    }

    setGameFilters(gameFilters.isHideBuddiesOnlyGames(), gameFilters.isHideIgnoredUserGames(),
                   gameFilters.isHideFullGames(), gameFilters.isHideGamesThatStarted(),
                   gameFilters.isHidePasswordProtectedGames(), gameFilters.isHideNotBuddyCreatedGames(),
                   gameFilters.isHideOpenDecklistGames(), gameFilters.getGameNameFilter(),
                   gameFilters.getCreatorNameFilters(), newGameTypeFilter, gameFilters.getMinPlayers(),
                   gameFilters.getMaxPlayers(), gameFilters.getMaxGameAge(),
                   gameFilters.isShowOnlyIfSpectatorsCanWatch(), gameFilters.isShowSpectatorPasswordProtected(),
                   gameFilters.isShowOnlyIfSpectatorsCanChat(), gameFilters.isShowOnlyIfSpectatorsCanSeeHands());
}

void GamesProxyModel::saveFilterParameters(const QMap<int, QString> &allGameTypes)
{
    GameFiltersSettings &gameFilters = SettingsCache::instance().gameFilters();
    gameFilters.setHideBuddiesOnlyGames(hideBuddiesOnlyGames);
    gameFilters.setHideFullGames(hideFullGames);
    gameFilters.setHideGamesThatStarted(hideGamesThatStarted);
    gameFilters.setHidePasswordProtectedGames(hidePasswordProtectedGames);
    gameFilters.setHideIgnoredUserGames(hideIgnoredUserGames);
    gameFilters.setHideNotBuddyCreatedGames(hideNotBuddyCreatedGames);
    gameFilters.setHideOpenDecklistGames(hideOpenDecklistGames);
    gameFilters.setGameNameFilter(gameNameFilter);
    gameFilters.setCreatorNameFilters(creatorNameFilters);

    QMapIterator<int, QString> gameTypeIterator(allGameTypes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();
        bool enabled = gameTypeFilter.contains(gameTypeIterator.key());
        gameFilters.setGameTypeEnabled(gameTypeIterator.value(), enabled);
    }

    gameFilters.setMinPlayers(maxPlayersFilterMin);
    gameFilters.setMaxPlayers(maxPlayersFilterMax);
    gameFilters.setMaxGameAge(maxGameAge);

    gameFilters.setShowOnlyIfSpectatorsCanWatch(showOnlyIfSpectatorsCanWatch);
    gameFilters.setShowSpectatorPasswordProtected(showSpectatorPasswordProtected);
    gameFilters.setShowOnlyIfSpectatorsCanChat(showOnlyIfSpectatorsCanChat);
    gameFilters.setShowOnlyIfSpectatorsCanSeeHands(showOnlyIfSpectatorsCanSeeHands);
}

bool GamesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    return filterAcceptsRow(sourceRow);
}

bool GamesProxyModel::filterAcceptsRow(int sourceRow) const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    static const QDate epochDate = QDateTime::fromSecsSinceEpoch(0, QTimeZone::UTC).date();
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    static const QDate epochDate = QDateTime::fromSecsSinceEpoch(0, Qt::UTC).date();
#else
    static const QDate epochDate = QDateTime::fromTime_t(0, Qt::UTC).date();
#endif
    auto *model = qobject_cast<GamesModel *>(sourceModel());
    if (!model)
        return false;

    const ServerInfo_Game &game = model->getGame(sourceRow);

    if (hideBuddiesOnlyGames && game.only_buddies()) {
        return false;
    }
    if (hideOpenDecklistGames && game.share_decklists_on_load()) {
        return false;
    }
    if (hideIgnoredUserGames && userListProxy->isUserIgnored(QString::fromStdString(game.creator_info().name()))) {
        return false;
    }
    if (hideNotBuddyCreatedGames && !userListProxy->isUserBuddy(QString::fromStdString(game.creator_info().name()))) {
        return false;
    }
    if (hideFullGames && game.player_count() == game.max_players())
        return false;
    if (hideGamesThatStarted && game.started())
        return false;
    if (!userListProxy->isOwnUserRegistered())
        if (game.only_registered())
            return false;
    if (hidePasswordProtectedGames && game.with_password())
        return false;
    if (!gameNameFilter.isEmpty())
        if (!QString::fromStdString(game.description()).contains(gameNameFilter, Qt::CaseInsensitive))
            return false;
    if (!creatorNameFilters.isEmpty()) {
        bool found = false;
        for (const auto &createNameFilter : creatorNameFilters) {
            if (QString::fromStdString(game.creator_info().name()).contains(createNameFilter, Qt::CaseInsensitive)) {
                found = true;
            }
        }
        if (!found) {
            return false;
        }
    }

    QSet<int> gameTypes;
    for (int i = 0; i < game.game_types_size(); ++i)
        gameTypes.insert(game.game_types(i));
    if (!gameTypeFilter.isEmpty() && gameTypes.intersect(gameTypeFilter).isEmpty())
        return false;

    if (game.max_players() < maxPlayersFilterMin)
        return false;
    if (game.max_players() > maxPlayersFilterMax)
        return false;

    if (maxGameAge.isValid()) {
        QDateTime now = QDateTime::currentDateTimeUtc();
        qint64 signed_start_time = game.start_time();      // cast to 64 bit value to allow signed value
        QDateTime total = now.addSecs(-signed_start_time); // a 32 bit value would wrap at 2038-1-19
        // games shouldn't have negative ages but we'll not filter them
        // because qtime wraps after a day we consider all games older than a day to be too old
        if (total.isValid() && total.date() >= epochDate && (total.date() > epochDate || total.time() > maxGameAge)) {
            return false;
        }
    }

    if (showOnlyIfSpectatorsCanWatch) {
        if (!game.spectators_allowed())
            return false;
        if (!showSpectatorPasswordProtected && game.spectators_need_password())
            return false;
        if (showOnlyIfSpectatorsCanChat && !game.spectators_can_chat())
            return false;
        if (showOnlyIfSpectatorsCanSeeHands && !game.spectators_omniscient())
            return false;
    }
    return true;
}

void GamesProxyModel::refresh()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
}
