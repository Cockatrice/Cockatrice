#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include "gametypemap.h"
#include "pb/serverinfo_game.pb.h"
#include "tab_supervisor.h"

#include <QAbstractTableModel>
#include <QList>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QStringList>

class GamesModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    QList<ServerInfo_Game> gameList;
    QMap<int, QString> rooms;
    QMap<int, GameTypeMap> gameTypes;

    static const int NUM_COLS = 8;
    static const int SECS_PER_MIN = 60;
    static const int SECS_PER_TEN_MIN = 600;
    static const int SECS_PER_HOUR = 3600;

public:
    static const int SORT_ROLE = Qt::UserRole + 1;

    GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        return parent.isValid() ? 0 : gameList.size();
    }
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const
    {
        return NUM_COLS;
    }
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    static const QString getGameCreatedString(const int secs);
    const ServerInfo_Game &getGame(int row);

    /**
     * Update game list with a (possibly new) game.
     */
    void updateGameList(const ServerInfo_Game &game);

    int roomColIndex()
    {
        return 0;
    }
    int startTimeColIndex()
    {
        return 1;
    }

    const QMap<int, GameTypeMap> &getGameTypes()
    {
        return gameTypes;
    }
};

class ServerInfo_User;

enum MaxGameAge
{
    NO_MAX_AGE = 0,
    FIVE_MINUTES = 1,
    TEN_MINUTES = 2,
    THIRTY_MINUTES = 3,
    ONE_HOUR = 4,
    TWO_HOURS = 5
};

class GamesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
private:
    bool ownUserIsRegistered;
    const TabSupervisor *tabSupervisor;

    // If adding any additional filters, make sure to update:
    // - GamesProxyModel()
    // - resetFilterParameters()
    // - areFilterParametersSetToDefaults()
    // - loadFilterParameters()
    // - saveFilterParameters()
    // - filterAcceptsRow()
    bool showBuddiesOnlyGames;
    bool hideIgnoredUserGames;
    bool unavailableGamesVisible;
    bool showPasswordProtectedGames;
    QString gameNameFilter, creatorNameFilter;
    QSet<int> gameTypeFilter;
    int maxPlayersFilterMin, maxPlayersFilterMax;
    MaxGameAge maxGameAge;

    static const bool DEFAULT_UNAVAILABLE_GAMES_VISIBLE = false;
    static const bool DEFAULT_SHOW_PASSWORD_PROTECTED_GAMES = true;
    static const bool DEFAULT_SHOW_BUDDIES_ONLY_GAMES = true;
    static const bool DEFAULT_HIDE_IGNORED_USER_GAMES = false;
    static const int DEFAULT_MAX_PLAYERS_MIN = 1;
    static const int DEFAULT_MAX_PLAYERS_MAX = 99;
    static const MaxGameAge DEFAULT_MAX_GAME_AGE = TWO_HOURS;

    static const int SECS_PER_MIN = 60;
    static const int SECS_PER_HOUR = 3600;

public:
    GamesProxyModel(QObject *parent = nullptr, const TabSupervisor *_tabSupervisor = nullptr);

    bool getShowBuddiesOnlyGames() const
    {
        return showBuddiesOnlyGames;
    }
    void setShowBuddiesOnlyGames(bool _showBuddiesOnlyGames);
    bool getHideIgnoredUserGames() const
    {
        return hideIgnoredUserGames;
    }
    void setHideIgnoredUserGames(bool _hideIgnoredUserGames);
    bool getUnavailableGamesVisible() const
    {
        return unavailableGamesVisible;
    }
    void setUnavailableGamesVisible(bool _unavailableGamesVisible);
    bool getShowPasswordProtectedGames() const
    {
        return showPasswordProtectedGames;
    }
    void setShowPasswordProtectedGames(bool _showPasswordProtectedGames);
    QString getGameNameFilter() const
    {
        return gameNameFilter;
    }
    void setGameNameFilter(const QString &_gameNameFilter);
    QString getCreatorNameFilter() const
    {
        return creatorNameFilter;
    }
    void setCreatorNameFilter(const QString &_creatorNameFilter);
    QSet<int> getGameTypeFilter() const
    {
        return gameTypeFilter;
    }
    void setGameTypeFilter(const QSet<int> &_gameTypeFilter);
    int getMaxPlayersFilterMin() const
    {
        return maxPlayersFilterMin;
    }
    int getMaxPlayersFilterMax() const
    {
        return maxPlayersFilterMax;
    }
    void setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax);
    MaxGameAge getMaxGameAge() const
    {
        return maxGameAge;
    }
    void setMaxGameAge(int _maxGameAge);
    static int getMaxGameAgeInSeconds(MaxGameAge maxGameAge);
    static const QStringList getMaxGameAgeOptions();

    int getNumFilteredGames() const;
    void resetFilterParameters();
    bool areFilterParametersSetToDefaults() const;
    void loadFilterParameters(const QMap<int, QString> &allGameTypes);
    void saveFilterParameters(const QMap<int, QString> &allGameTypes);
    void refresh();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool filterAcceptsRow(int sourceRow) const;
};

#endif
