#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QSet>
#include "gametypemap.h"
#include "pb/serverinfo_game.pb.h"

class ServerInfo_User;

class GamesModel : public QAbstractTableModel {
    Q_OBJECT
private:
    QList<ServerInfo_Game> gameList;
    QMap<int, QString> rooms;
    QMap<int, GameTypeMap> gameTypes;

    static const int NUM_COLS = 9;
public:
    static const int SORT_ROLE = Qt::UserRole+1;

    GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const { return parent.isValid() ? 0 : gameList.size(); }
    int columnCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return NUM_COLS; }
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    const ServerInfo_Game &getGame(int row);

    /**
     * Update game list with a (possibly new) game.
     */
    void updateGameList(const ServerInfo_Game &game);

    int roomColIndex() { return 0; }
    int startTimeColIndex() { return 1; }

    const QMap<int, GameTypeMap> &getGameTypes() { return gameTypes; }
};

class GamesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
private:
    ServerInfo_User *ownUser;
    bool unavailableGamesVisible;
    bool passwordProtectedGamesVisible;
    QString gameNameFilter, creatorNameFilter;
    QSet<int> gameTypeFilter;
    int maxPlayersFilterMin, maxPlayersFilterMax;
public:
    GamesProxyModel(QObject *parent = 0, ServerInfo_User *_ownUser = 0);

    bool getUnavailableGamesVisible() const { return unavailableGamesVisible; }
    void setUnavailableGamesVisible(bool _unavailableGamesVisible);
    bool getPasswordProtectedGamesVisible() const { return passwordProtectedGamesVisible; }
    void setPasswordProtectedGamesVisible(bool _passwordProtectedGamesVisible);
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
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif
