#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include "game_type_map.h"

#include <QList>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTime>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>

class UserListProxy;

/**
 * @class GamesModel
 * @ingroup Lobby
 * @brief Model storing all available games for display in a QTreeView or QTableView.
 *
 * Provides access to game information, supports sorting by different columns,
 * and updates when new game data is received from the server.
 */
class GamesModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    QList<ServerInfo_Game> gameList;  /**< List of games currently displayed. */
    QMap<int, QString> rooms;         /**< Map of room IDs to room names. */
    QMap<int, GameTypeMap> gameTypes; /**< Map of room IDs to available game types. */

    static const int NUM_COLS = 8; /**< Number of columns in the table. */

public:
    static const int SORT_ROLE = Qt::UserRole + 1; /**< Role used for sorting. */

    /**
     * @brief Constructs a GamesModel.
     * @param _rooms Mapping of room IDs to room names.
     * @param _gameTypes Mapping of room IDs to their available game types.
     * @param parent Parent QObject.
     */
    GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : gameList.size();
    }

    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override
    {
        return NUM_COLS;
    }

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Formats the game creation time into a human-readable string.
     * @param secs Number of seconds since the game started.
     * @return Short string representing game age (e.g., "new", ">5 min", ">2 hr").
     */
    static const QString getGameCreatedString(const int secs);

    /**
     * @brief Returns a reference to a specific game by row index.
     * @param row Row index in the table.
     * @return Reference to the ServerInfo_Game at the given row.
     */
    const ServerInfo_Game &getGame(int row);

    /**
     * @brief Updates the game list with a new or updated game.
     * @param game The ServerInfo_Game object to add or update.
     */
    void updateGameList(const ServerInfo_Game &game);

    /**
     * @brief Returns the index of the room column.
     */
    int roomColIndex()
    {
        return 0;
    }

    /**
     * @brief Returns the index of the start time column.
     */
    int startTimeColIndex()
    {
        return 1;
    }

    /**
     * @brief Returns the map of game types per room.
     */
    const QMap<int, GameTypeMap> &getGameTypes()
    {
        return gameTypes;
    }
};

/**
 * @class GamesProxyModel
 * @ingroup Lobby
 * @brief Proxy model for filtering and sorting the GamesModel based on user preferences.
 *
 * Supports filtering games based on buddies-only, ignored users, password protection,
 * game types, creator, age, player count, and spectator permissions.
 */
class GamesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
private:
    const UserListProxy *userListProxy; /**< Proxy for checking user ignore/buddy lists. */

    // If adding any additional filters, make sure to update:
    // - GamesProxyModel()
    // - resetFilterParameters()
    // - areFilterParametersSetToDefaults()
    // - loadFilterParameters()
    // - saveFilterParameters()
    // - filterAcceptsRow()
    bool hideBuddiesOnlyGames;
    bool hideIgnoredUserGames;
    bool hideFullGames;
    bool hideGamesThatStarted;
    bool hidePasswordProtectedGames;
    bool hideNotBuddyCreatedGames;
    bool hideOpenDecklistGames;
    QString gameNameFilter;
    QStringList creatorNameFilters;
    QSet<int> gameTypeFilter;
    quint32 maxPlayersFilterMin, maxPlayersFilterMax;
    QTime maxGameAge;
    bool showOnlyIfSpectatorsCanWatch;
    bool showSpectatorPasswordProtected;
    bool showOnlyIfSpectatorsCanChat;
    bool showOnlyIfSpectatorsCanSeeHands;

public:
    /**
     * @brief Constructs a GamesProxyModel.
     * @param parent Parent QObject.
     * @param _userListProxy Proxy for accessing ignore/buddy lists.
     */
    explicit GamesProxyModel(QObject *parent = nullptr, const UserListProxy *_userListProxy = nullptr);

    // Getters for filter parameters
    [[nodiscard]] bool getHideBuddiesOnlyGames() const
    {
        return hideBuddiesOnlyGames;
    }
    [[nodiscard]] bool getHideIgnoredUserGames() const
    {
        return hideIgnoredUserGames;
    }
    [[nodiscard]] bool getHideFullGames() const
    {
        return hideFullGames;
    }
    [[nodiscard]] bool getHideGamesThatStarted() const
    {
        return hideGamesThatStarted;
    }
    [[nodiscard]] bool getHidePasswordProtectedGames() const
    {
        return hidePasswordProtectedGames;
    }
    [[nodiscard]] bool getHideNotBuddyCreatedGames() const
    {
        return hideNotBuddyCreatedGames;
    }
    [[nodiscard]] bool getHideOpenDecklistGames() const
    {
        return hideOpenDecklistGames;
    }
    [[nodiscard]] QString getGameNameFilter() const
    {
        return gameNameFilter;
    }
    [[nodiscard]] QStringList getCreatorNameFilters() const
    {
        return creatorNameFilters;
    }
    [[nodiscard]] QSet<int> getGameTypeFilter() const
    {
        return gameTypeFilter;
    }
    [[nodiscard]] int getMaxPlayersFilterMin() const
    {
        return maxPlayersFilterMin;
    }
    [[nodiscard]] int getMaxPlayersFilterMax() const
    {
        return maxPlayersFilterMax;
    }
    [[nodiscard]] const QTime &getMaxGameAge() const
    {
        return maxGameAge;
    }
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanWatch() const
    {
        return showOnlyIfSpectatorsCanWatch;
    }
    [[nodiscard]] bool getShowSpectatorPasswordProtected() const
    {
        return showSpectatorPasswordProtected;
    }
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanChat() const
    {
        return showOnlyIfSpectatorsCanChat;
    }
    [[nodiscard]] bool getShowOnlyIfSpectatorsCanSeeHands() const
    {
        return showOnlyIfSpectatorsCanSeeHands;
    }

    /**
     * @brief Sets all game filters at once.
     */
    void setGameFilters(bool _hideBuddiesOnlyGames,
                        bool _hideIgnoredUserGames,
                        bool _hideFullGames,
                        bool _hideGamesThatStarted,
                        bool _hidePasswordProtectedGames,
                        bool _hideNotBuddyCreatedGames,
                        bool _hideOpenDecklistGames,
                        const QString &_gameNameFilter,
                        const QStringList &_creatorNameFilter,
                        const QSet<int> &_gameTypeFilter,
                        int _maxPlayersFilterMin,
                        int _maxPlayersFilterMax,
                        const QTime &_maxGameAge,
                        bool _showOnlyIfSpectatorsCanWatch,
                        bool _showSpectatorPasswordProtected,
                        bool _showOnlyIfSpectatorsCanChat,
                        bool _showOnlyIfSpectatorsCanSeeHands);

    // Individual setters
    void setHideBuddiesOnlyGames(bool value)
    {
        hideBuddiesOnlyGames = value;
        refresh();
    }
    void setHideIgnoredUserGames(bool value)
    {
        hideIgnoredUserGames = value;
        refresh();
    }
    void setHideFullGames(bool value)
    {
        hideFullGames = value;
        refresh();
    }
    void setHideGamesThatStarted(bool value)
    {
        hideGamesThatStarted = value;
        refresh();
    }
    void setHidePasswordProtectedGames(bool value)
    {
        hidePasswordProtectedGames = value;
        refresh();
    }
    void setHideNotBuddyCreatedGames(bool value)
    {
        hideNotBuddyCreatedGames = value;
        refresh();
    }
    void setHideOpenDecklistGames(bool value)
    {
        hideOpenDecklistGames = value;
        refresh();
    }
    void setGameNameFilter(const QString &value)
    {
        gameNameFilter = value;
        refresh();
    }
    void setCreatorNameFilters(const QStringList &values)
    {
        creatorNameFilters = values;
        refresh();
    }
    void setGameTypeFilter(const QSet<int> &value)
    {
        gameTypeFilter = value;
        refresh();
    }
    void setMaxPlayersFilterMin(int value)
    {
        maxPlayersFilterMin = value;
        refresh();
    }
    void setMaxPlayersFilterMax(int value)
    {
        maxPlayersFilterMax = value;
        refresh();
    }
    void setMaxGameAge(const QTime &value)
    {
        maxGameAge = value;
        refresh();
    }
    void setShowOnlyIfSpectatorsCanWatch(bool value)
    {
        showOnlyIfSpectatorsCanWatch = value;
        refresh();
    }
    void setShowSpectatorPasswordProtected(bool value)
    {
        showSpectatorPasswordProtected = value;
        refresh();
    }
    void setShowOnlyIfSpectatorsCanChat(bool value)
    {
        showOnlyIfSpectatorsCanChat = value;
        refresh();
    }
    void setShowOnlyIfSpectatorsCanSeeHands(bool value)
    {
        showOnlyIfSpectatorsCanSeeHands = value;
        refresh();
    }

    /**
     * @brief Returns the number of games filtered out by the current filter.
     */
    [[nodiscard]] int getNumFilteredGames() const;

    /**
     * @brief Resets all filter parameters to default values.
     */
    void resetFilterParameters();

    /**
     * @brief Returns true if all filter parameters are set to their defaults.
     */
    [[nodiscard]] bool areFilterParametersSetToDefaults() const;

    /**
     * @brief Loads filter parameters from persistent settings.
     * @param allGameTypes Mapping of all game types by room ID.
     */
    void loadFilterParameters(const QMap<int, QString> &allGameTypes);

    /**
     * @brief Saves filter parameters to persistent settings.
     * @param allGameTypes Mapping of all game types by room ID.
     */
    void saveFilterParameters(const QMap<int, QString> &allGameTypes);

    /**
     * @brief Refreshes the proxy model (re-applies filters).
     */
    void refresh();

protected:
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow) const;
};

#endif
