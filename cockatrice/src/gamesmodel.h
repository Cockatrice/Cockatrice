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
#include <QTime>

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
    bool showFullGames;
    bool showGamesThatStarted;
    bool showPasswordProtectedGames;
    QString gameNameFilter, creatorNameFilter;
    QSet<int> gameTypeFilter;
    quint32 maxPlayersFilterMin, maxPlayersFilterMax;
    QTime maxGameAge;
    bool showOnlyIfSpectatorsCanWatch, showSpectatorPasswordProtected, showOnlyIfSpectatorsCanChat,
        showOnlyIfSpectatorsCanSeeHands;

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
    bool getShowFullGames() const
    {
        return showFullGames;
    }
    void setShowFullGames(bool _showFullGames);
    bool getShowGamesThatStarted() const
    {
        return showGamesThatStarted;
    }
    void setShowGamesThatStarted(bool _showGamesThatStarted);
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
    const QTime &getMaxGameAge() const
    {
        return maxGameAge;
    }
    void setMaxGameAge(const QTime &_maxGameAge);
    bool getShowOnlyIfSpectatorsCanWatch() const
    {
        return showOnlyIfSpectatorsCanWatch;
    }
    void setShowOnlyIfSpectatorsCanWatch(bool _showOnlyIfSpectatorsCanWatch);
    bool getShowSpectatorPasswordProtected() const
    {
        return showSpectatorPasswordProtected;
    }
    void setShowSpectatorPasswordProtected(bool _showSpectatorPasswordProtected);
    bool getShowOnlyIfSpectatorsCanChat() const
    {
        return showOnlyIfSpectatorsCanChat;
    }
    void setShowOnlyIfSpectatorsCanChat(bool _showOnlyIfSpectatorsCanChat);
    bool getShowOnlyIfSpectatorsCanSeeHands() const
    {
        return showOnlyIfSpectatorsCanSeeHands;
    }
    void setShowOnlyIfSpectatorsCanSeeHands(bool _showOnlyIfSpectatorsCanSeeHands);

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
