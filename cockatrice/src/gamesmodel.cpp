#include "gamesmodel.h"

#include "pb/serverinfo_game.pb.h"
#include "pixmapgenerator.h"
#include "settingscache.h"
#include "tab_account.h"
#include "userlist.h"

#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QStringList>

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

const bool DEFAULT_SHOW_FULL_GAMES = false;
const bool DEFAULT_SHOW_GAMES_THAT_STARTED = false;
const bool DEFAULT_SHOW_PASSWORD_PROTECTED_GAMES = true;
const bool DEFAULT_SHOW_BUDDIES_ONLY_GAMES = true;
const bool DEFAULT_HIDE_IGNORED_USER_GAMES = false;
const bool DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_WATCH = false;
const bool DEFAULT_SHOW_SPECTATOR_PASSWORD_PROTECTED = true;
const bool DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_CHAT = false;
const bool DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_SEE_HANDS = false;
const int DEFAULT_MAX_PLAYERS_MIN = 1;
const int DEFAULT_MAX_PLAYERS_MAX = 99;
constexpr QTime DEFAULT_MAX_GAME_AGE = QTime();

const QString GamesModel::getGameCreatedString(const int secs)
{

    QString ret;
    if (secs < SECS_PER_MIN * 2) // for first min we display "New"
        ret = tr("New");
    else if (secs < SECS_PER_MIN * 10) // from 2 - 10 mins we show the mins
        ret = QString("%1 min").arg(QString::number(secs / SECS_PER_MIN));
    else if (secs < SECS_PER_MIN * 60) { // from 10 mins to 1h we aggregate every 10 mins
        int unitOfTen = secs / SECS_PER_TEN_MIN;
        QString str = "%1%2";
        ret = str.arg(QString::number(unitOfTen), "0+ min");
    } else { // from 1 hr onward we show hrs
        int hours = secs / SECS_PER_HOUR;
        if (secs % SECS_PER_HOUR >= SECS_PER_MIN * 30) // if the room is open for 1hr 30 mins, we round to 2hrs
            ++hours;
        ret = QString("%1+ h").arg(QString::number(hours));
    }
    return ret;
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
                    QDateTime then = QDateTime::fromSecsSinceEpoch(gameentry.start_time(), Qt::UTC);
#else
                    QDateTime then = QDateTime::fromTime_t(gameentry.start_time(), Qt::UTC);
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
                    QPixmap avatarPixmap = UserLevelPixmapGenerator::generatePixmap(
                        13, (UserLevelFlags)gameentry.creator_info().user_level(), false,
                        QString::fromStdString(gameentry.creator_info().privlevel()));
                    return QIcon(avatarPixmap);
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
    if (game.player_count() <= 0)
        return;
    beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
    gameList.append(game);
    endInsertRows();
}

GamesProxyModel::GamesProxyModel(QObject *parent, const TabSupervisor *_tabSupervisor)
    : QSortFilterProxyModel(parent), ownUserIsRegistered(_tabSupervisor->isOwnUserRegistered()),
      tabSupervisor(_tabSupervisor)
{
    resetFilterParameters();
    setSortRole(GamesModel::SORT_ROLE);
    setDynamicSortFilter(true);
}

void GamesProxyModel::setShowBuddiesOnlyGames(bool _showBuddiesOnlyGames)
{
    showBuddiesOnlyGames = _showBuddiesOnlyGames;
    invalidateFilter();
}

void GamesProxyModel::setHideIgnoredUserGames(bool _hideIgnoredUserGames)
{
    hideIgnoredUserGames = _hideIgnoredUserGames;
    invalidateFilter();
}

void GamesProxyModel::setShowFullGames(bool _showFullGames)
{
    showFullGames = _showFullGames;
    invalidateFilter();
}

void GamesProxyModel::setShowGamesThatStarted(bool _showGamesThatStarted)
{
    showGamesThatStarted = _showGamesThatStarted;
    invalidateFilter();
}

void GamesProxyModel::setShowPasswordProtectedGames(bool _showPasswordProtectedGames)
{
    showPasswordProtectedGames = _showPasswordProtectedGames;
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

void GamesProxyModel::setMaxGameAge(const QTime &_maxGameAge)
{
    maxGameAge = _maxGameAge;
    invalidateFilter();
}

void GamesProxyModel::setShowOnlyIfSpectatorsCanWatch(bool _showOnlyIfSpectatorsCanWatch)
{
    showOnlyIfSpectatorsCanWatch = _showOnlyIfSpectatorsCanWatch;
    invalidateFilter();
}

void GamesProxyModel::setShowSpectatorPasswordProtected(bool _showSpectatorPasswordProtected)
{
    showSpectatorPasswordProtected = _showSpectatorPasswordProtected;
    invalidateFilter();
}

void GamesProxyModel::setShowOnlyIfSpectatorsCanChat(bool _showOnlyIfSpectatorsCanChat)
{
    showOnlyIfSpectatorsCanChat = _showOnlyIfSpectatorsCanChat;
    invalidateFilter();
}

void GamesProxyModel::setShowOnlyIfSpectatorsCanSeeHands(bool _showOnlyIfSpectatorsCanSeeHands)
{
    showOnlyIfSpectatorsCanSeeHands = _showOnlyIfSpectatorsCanSeeHands;
    invalidateFilter();
}

int GamesProxyModel::getNumFilteredGames() const
{
    GamesModel *model = qobject_cast<GamesModel *>(sourceModel());
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
    showFullGames = DEFAULT_SHOW_FULL_GAMES;
    showGamesThatStarted = DEFAULT_SHOW_GAMES_THAT_STARTED;
    showPasswordProtectedGames = DEFAULT_SHOW_PASSWORD_PROTECTED_GAMES;
    showBuddiesOnlyGames = DEFAULT_SHOW_BUDDIES_ONLY_GAMES;
    hideIgnoredUserGames = DEFAULT_HIDE_IGNORED_USER_GAMES;
    gameNameFilter = QString();
    creatorNameFilter = QString();
    gameTypeFilter.clear();
    maxPlayersFilterMin = DEFAULT_MAX_PLAYERS_MIN;
    maxPlayersFilterMax = DEFAULT_MAX_PLAYERS_MAX;
    maxGameAge = DEFAULT_MAX_GAME_AGE;
    showOnlyIfSpectatorsCanWatch = DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_WATCH;
    showSpectatorPasswordProtected = DEFAULT_SHOW_SPECTATOR_PASSWORD_PROTECTED;
    showOnlyIfSpectatorsCanChat = DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_CHAT;
    showOnlyIfSpectatorsCanSeeHands = DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_SEE_HANDS;

    invalidateFilter();
}

bool GamesProxyModel::areFilterParametersSetToDefaults() const
{
    return showFullGames == DEFAULT_SHOW_FULL_GAMES && showGamesThatStarted == DEFAULT_SHOW_GAMES_THAT_STARTED &&
           showPasswordProtectedGames == DEFAULT_SHOW_PASSWORD_PROTECTED_GAMES &&
           showBuddiesOnlyGames == DEFAULT_SHOW_BUDDIES_ONLY_GAMES &&
           hideIgnoredUserGames == DEFAULT_HIDE_IGNORED_USER_GAMES && gameNameFilter.isEmpty() &&
           creatorNameFilter.isEmpty() && gameTypeFilter.isEmpty() && maxPlayersFilterMin == DEFAULT_MAX_PLAYERS_MIN &&
           maxPlayersFilterMax == DEFAULT_MAX_PLAYERS_MAX && maxGameAge == DEFAULT_MAX_GAME_AGE &&
           showOnlyIfSpectatorsCanWatch == DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_WATCH &&
           showSpectatorPasswordProtected == DEFAULT_SHOW_SPECTATOR_PASSWORD_PROTECTED &&
           showOnlyIfSpectatorsCanChat == DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_CHAT &&
           showOnlyIfSpectatorsCanSeeHands == DEFAULT_SHOW_ONLY_IF_SPECTATORS_CAN_SEE_HANDS;
}

void GamesProxyModel::loadFilterParameters(const QMap<int, QString> &allGameTypes)
{
    GameFiltersSettings &gameFilters = SettingsCache::instance().gameFilters();
    showFullGames = gameFilters.isShowFullGames();
    showGamesThatStarted = gameFilters.isShowGamesThatStarted();
    showPasswordProtectedGames = gameFilters.isShowPasswordProtectedGames();
    hideIgnoredUserGames = gameFilters.isHideIgnoredUserGames();
    showBuddiesOnlyGames = gameFilters.isShowBuddiesOnlyGames();
    gameNameFilter = gameFilters.getGameNameFilter();
    creatorNameFilter = gameFilters.getCreatorNameFilter();
    maxPlayersFilterMin = gameFilters.getMinPlayers();
    maxPlayersFilterMax = gameFilters.getMaxPlayers();
    maxGameAge = gameFilters.getMaxGameAge();
    showOnlyIfSpectatorsCanWatch = gameFilters.isShowOnlyIfSpectatorsCanWatch();
    showSpectatorPasswordProtected = gameFilters.isShowSpectatorPasswordProtected();
    showOnlyIfSpectatorsCanChat = gameFilters.isShowOnlyIfSpectatorsCanChat();
    showOnlyIfSpectatorsCanSeeHands = gameFilters.isShowOnlyIfSpectatorsCanSeeHands();

    QMapIterator<int, QString> gameTypesIterator(allGameTypes);
    while (gameTypesIterator.hasNext()) {
        gameTypesIterator.next();
        if (gameFilters.isGameTypeEnabled(gameTypesIterator.value())) {
            gameTypeFilter.insert(gameTypesIterator.key());
        }
    }

    invalidateFilter();
}

void GamesProxyModel::saveFilterParameters(const QMap<int, QString> &allGameTypes)
{
    GameFiltersSettings &gameFilters = SettingsCache::instance().gameFilters();
    gameFilters.setShowBuddiesOnlyGames(showBuddiesOnlyGames);
    gameFilters.setShowFullGames(showFullGames);
    gameFilters.setShowGamesThatStarted(showGamesThatStarted);
    gameFilters.setShowPasswordProtectedGames(showPasswordProtectedGames);
    gameFilters.setHideIgnoredUserGames(hideIgnoredUserGames);
    gameFilters.setGameNameFilter(gameNameFilter);
    gameFilters.setCreatorNameFilter(creatorNameFilter);

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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    static const QDate epochDate = QDateTime::fromSecsSinceEpoch(0, Qt::UTC).date();
#else
    static const QDate epochDate = QDateTime::fromTime_t(0, Qt::UTC).date();
#endif
    auto *model = qobject_cast<GamesModel *>(sourceModel());
    if (!model)
        return false;

    const ServerInfo_Game &game = model->getGame(sourceRow);

    if (!showBuddiesOnlyGames && game.only_buddies()) {
        return false;
    }
    if (hideIgnoredUserGames && tabSupervisor->getUserListsTab()->getIgnoreList()->getUsers().contains(
                                    QString::fromStdString(game.creator_info().name()))) {
        return false;
    }
    if (!showFullGames && game.player_count() == game.max_players())
        return false;
    if (!showGamesThatStarted && game.started())
        return false;
    if (!ownUserIsRegistered)
        if (game.only_registered())
            return false;
    if (!showPasswordProtectedGames && game.with_password())
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
    invalidateFilter();
}
