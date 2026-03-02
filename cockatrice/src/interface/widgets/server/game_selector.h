#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include "game_selector_quick_filter_toolbar.h"
#include "game_type_map.h"

#include <QGroupBox>
#include <libcockatrice/protocol/pb/commands.pb.h>
#include <libcockatrice/protocol/pb/event_add_to_list.pb.h>
#include <libcockatrice/protocol/pb/event_remove_from_list.pb.h>

class QTreeView;
class GamesModel;
class GamesProxyModel;
class QPushButton;
class QCheckBox;
class QLabel;
class AbstractClient;
class TabSupervisor;
class TabRoom;
class ServerInfo_Game;
class Response;

/**
 * @class GameSelector
 * @ingroup Lobby
 * @brief Provides a widget for displaying, filtering, joining, spectating, and creating games in a room.
 *
 * The GameSelector displays all available games in a QTreeView. It supports filtering,
 * creating, joining, spectating, and viewing game details. Integrates with TabSupervisor
 * and TabRoom for room and game management.
 */
class GameSelector : public QGroupBox
{
    Q_OBJECT
private slots:
    /**
     * @brief Opens a dialog to set filters for the game list.
     *
     * Updates the proxy model with selected filter parameters and refreshes the displayed game list.
     */
    void actSetFilter();

    /**
     * @brief Clears all filters applied to the game list.
     *
     * Resets the proxy model to show all games.
     */
    void actClearFilter();

    /**
     * @brief Opens the dialog to create a new game in the current room.
     */
    void actCreate();

    /**
     * @brief Joins the currently selected game as a player.
     */
    void actJoin();
    /**
     * @brief Joins the currently selected game as a judge.
     */
    void actJoinAsJudge();

    /**
     * @brief Joins the currently selected game as a spectator.
     */
    void actJoinAsSpectator();
    void actJoinAsJudgeSpectator();

    /**
     * @brief Shows the custom context menu for a game when right-clicked.
     * @param point The point at which the context menu is requested.
     */
    void customContextMenu(const QPoint &point);

    /**
     * @brief Slot called when the selected game changes.
     * @param current The currently selected index.
     * @param previous The previously selected index.
     *
     * Updates the enabled/disabled state of buttons depending on the selected game.
     */
    void actSelectedGameChanged(const QModelIndex &current, const QModelIndex &previous);

    /**
     * @brief Processes server responses for join or spectate commands.
     * @param response The response from the server.
     *
     * Displays error messages for failed join/spectate attempts.
     */
    void checkResponse(const Response &response);

    /**
     * @brief Refreshes the game list when the ignore list is received from the server.
     * @param _ignoreList The list of users being ignored.
     */
    void ignoreListReceived(const QList<ServerInfo_User> &_ignoreList);

    /**
     * @brief Processes events where a user is added to a list (e.g., ignore or buddy).
     * @param event The event information.
     */
    void processAddToListEvent(const Event_AddToList &event);

    /**
     * @brief Processes events where a user is removed from a list (e.g., ignore or buddy).
     * @param event The event information.
     */
    void processRemoveFromListEvent(const Event_RemoveFromList &event);

signals:
    /**
     * @brief Emitted when a game has been successfully joined.
     * @param gameId The ID of the joined game.
     */
    void gameJoined(int gameId);

private:
    AbstractClient *client;       /**< The network client used to communicate with the server. */
    TabSupervisor *tabSupervisor; /**< Reference to TabSupervisor for managing tabs and rooms. */
    TabRoom *room;                /**< The current room. */

    QTreeView *gameListView;             /**< View widget for displaying the game list. */
    GamesModel *gameListModel;           /**< Model containing all games. */
    GamesProxyModel *gameListProxyModel; /**< Proxy model for filtering and sorting the game list. */

    GameSelectorQuickFilterToolBar *quickFilterToolBar;

    QPushButton *filterButton;               /**< Button to open the filter dialog. */
    QPushButton *clearFilterButton;          /**< Button to clear active filters. */
    QPushButton *createButton;               /**< Button to create a new game (only if room is set). */
    QPushButton *joinButton;                 /**< Button to join the selected game. */
    QPushButton *joinAsJudgeButton;          /**< Button to join the selected game as a judge. */
    QPushButton *spectateButton;             /**< Button to spectate the selected game. */
    QPushButton *joinAsJudgeSpectatorButton; /**< Button to join the selected game as a spectating judge. */

    const bool showFilters;  /**< Determines whether filter buttons are displayed. */
    GameTypeMap gameTypeMap; /**< Mapping of game types for the current room. */

    /**
     * @brief Updates the widget title to reflect the current number of displayed games.
     *
     * Shows the number of visible games versus total games if filters are enabled.
     */
    void updateTitle();

    /**
     * @brief Disables create/join/spectate buttons.
     */
    void disableButtons();

    /**
     * @brief Enables buttons for the currently selected game.
     */
    void enableButtons();

    /**
     * @brief Enables buttons for a specific game index.
     * @param current The index of the currently selected game.
     */
    void enableButtonsForIndex(const QModelIndex &current);

    /**
     * @brief Performs the join or spectate action for the currently selected game.
     * @param asSpectator True to join as a spectator, false to join as a player.
     * @param asJudge True to join as a judge, false to join as a player.
     *
     * Handles password prompts, overrides, and sending the join command to the server.
     */
    void joinGame(bool asSpectator = false, bool asJudge = false);

public:
    /**
     * @brief Constructs a GameSelector widget.
     * @param _client The network client used to communicate with the server.
     * @param _tabSupervisor Reference to TabSupervisor for managing tabs and rooms.
     * @param _room Pointer to the current room; nullptr if no room is selected.
     * @param _rooms Map of room IDs to room names.
     * @param _gameTypes Map of room IDs to their available game types.
     * @param restoresettings Whether to restore filter settings from previous sessions.
     * @param _showfilters Whether to display filter buttons.
     * @param parent Parent QWidget.
     */
    GameSelector(AbstractClient *_client,
                 TabSupervisor *_tabSupervisor,
                 TabRoom *_room,
                 const QMap<int, QString> &_rooms,
                 const QMap<int, GameTypeMap> &_gameTypes,
                 const bool restoresettings,
                 const bool _showfilters,
                 QWidget *parent = nullptr);

    /**
     * @brief Updates UI text for translation/localization.
     */
    void retranslateUi();

    /**
     * @brief Updates or adds a game entry in the list.
     * @param info The ServerInfo_Game object containing information about the game to update.
     */
    void processGameInfo(const ServerInfo_Game &info);
};

#endif
