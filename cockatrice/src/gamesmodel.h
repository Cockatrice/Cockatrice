#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QSet>
#include "gametypemap.h"
#include "pb/serverinfo_game.pb.h"

class GamesModel : public QAbstractTableModel {
Q_OBJECT
private:
    QList<ServerInfo_Game> gameList;
    QMap<int, QString> rooms;
    QMap<int, GameTypeMap> gameTypes;

    static const int NUM_COLS = 8;
    static const int SECS_PER_MIN  = 60;
    static const int SECS_PER_TEN_MIN = 600;
    static const int SECS_PER_HOUR = 3600;
public:
    static const int SORT_ROLE = Qt::UserRole+1;

    GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const { return parent.isValid() ? 0 : gameList.size(); }
    int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return NUM_COLS; }
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    const QString getGameCreatedString(const int secs) const;
    const ServerInfo_Game &getGame(int row);

    /**
     * Update game list with a (possibly new) game.
     */
    void updateGameList(const ServerInfo_Game &game);

    int roomColIndex() { return 0; }
    int startTimeColIndex() { return 1; }

    const QMap<int, GameTypeMap> &getGameTypes() { return gameTypes; }
};

class ServerInfo_User;

class GamesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
private:
    ServerInfo_User *ownUser;
    bool showBuddiesOnlyGames;
    bool unavailableGamesVisible;
    bool showPasswordProtectedGames;
    QString gameNameFilter, creatorNameFilter;
    QSet<int> gameTypeFilter;
    int maxPlayersFilterMin, maxPlayersFilterMax;

    static const int DEFAULT_MAX_PLAYERS_MAX = 99;   
public:
    GamesProxyModel(QObject *parent = 0, ServerInfo_User *_ownUser = 0);

    bool getShowBuddiesOnlyGames() const {return showBuddiesOnlyGames; }
    void setShowBuddiesOnlyGames(bool _showBuddiesOnlyGames);
    bool getUnavailableGamesVisible() const { return unavailableGamesVisible; }
    void setUnavailableGamesVisible(bool _unavailableGamesVisible);
    bool getShowPasswordProtectedGames() const { return showPasswordProtectedGames; }
    void setShowPasswordProtectedGames(bool _showPasswordProtectedGames);
    QString getGameNameFilter() const { return gameNameFilter; }
    void setGameNameFilter(const QString &_gameNameFilter);
    QString getCreatorNameFilter() const { return creatorNameFilter; }
    void setCreatorNameFilter(const QString &_creatorNameFilter);
    QSet<int> getGameTypeFilter() const { return gameTypeFilter; }
    void setGameTypeFilter(const QSet<int> &_gameTypeFilter);
    int getMaxPlayersFilterMin() const { return maxPlayersFilterMin; }
    int getMaxPlayersFilterMax() const { return maxPlayersFilterMax; }
    void setMaxPlayersFilter(int _maxPlayersFilterMin, int _maxPlayersFilterMax);
    void resetFilterParameters();
    void loadFilterParameters(const QMap<int, QString> &allGameTypes);
    void saveFilterParameters(const QMap<int, QString> &allGameTypes);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
