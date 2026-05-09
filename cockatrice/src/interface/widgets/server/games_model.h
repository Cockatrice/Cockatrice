#ifndef GAMESMODEL_H
#define GAMESMODEL_H

#include "game_filter_configs.h"
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
    GameFilterConfigs filters;

signals:
    void filtersChanged();

public:
    /**
     * @brief Constructs a GamesProxyModel.
     * @param parent Parent QObject.
     * @param _userListProxy Proxy for accessing ignore/buddy lists.
     */
    explicit GamesProxyModel(QObject *parent = nullptr, const UserListProxy *_userListProxy = nullptr);

    // Getters for filter parameters
    [[nodiscard]] const GameFilterConfigs &getFilters() const
    {
        return filters;
    }

    /**
     * @brief Sets all game filters at once.
     */
    void setGameFilters(const GameFilterConfigs &_filters);

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
