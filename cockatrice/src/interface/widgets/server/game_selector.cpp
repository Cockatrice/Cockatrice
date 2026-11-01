#include "game_selector.h"

#include "../../../client/settings/cache_settings.h"
#include "../../pixel_map_generator.h"
#include "../interface/widgets/dialogs/dlg_create_game.h"
#include "../interface/widgets/dialogs/dlg_filter_games.h"
#include "../interface/widgets/tabs/tab_room.h"
#include "../interface/widgets/tabs/tab_supervisor.h"
#include "../interface/widgets/utility/get_text_with_max.h"
#include "game_link.h"
#include "game_selector_quick_filter_toolbar.h"
#include "games_model.h"
#include "libcockatrice/protocol/pb/event_add_to_list.pb.h"
#include "libcockatrice/protocol/pb/event_remove_from_list.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user.pb.h"
#include "user/user_list_manager.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSize>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringLiteral>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGlobal>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_game.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/layouts_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <optional>
#include <qlogging.h>
#include <qnamespace.h>
#include <string>

class QModelIndex;
class QPoint;
class QWidget;
template <typename T> class QList;

GameSelector::GameSelector(AbstractClient *_client,
                           TabSupervisor *_tabSupervisor,
                           TabRoom *_room,
                           const QMap<int, QString> &_rooms,
                           const QMap<int, GameTypeMap> &_gameTypes,
                           const bool restoresettings,
                           const bool _showfilters,
                           QWidget *parent)
    : QGroupBox(parent), client(_client), tabSupervisor(_tabSupervisor), room(_room), showFilters(_showfilters)
{
    gameListView = new QTreeView;
    gameListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(gameListView, &QTreeView::customContextMenuRequested, this, &GameSelector::customContextMenu);

    gameListModel = new GamesModel(_rooms, _gameTypes, this);
    gameListProxyModel = new GamesProxyModel(this, tabSupervisor->getUserListManager());

    if (showFilters) {
        gameListProxyModel->setSourceModel(gameListModel);
        gameListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        gameListView->setModel(gameListProxyModel);
    } else {
        // User-game windows need sorting without applying the lobby's game filters.
        auto *sortModel = new QSortFilterProxyModel(this);
        sortModel->setSourceModel(gameListModel);
        sortModel->setSortRole(GamesModel::SORT_ROLE);
        sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        gameListView->setModel(sortModel);
    }
    gameListView->setIconSize(QSize(13, 13));
    gameListView->setSortingEnabled(true);
    gameListView->sortByColumn(gameListModel->startTimeColIndex(), Qt::AscendingOrder);
    gameListView->setAlternatingRowColors(true);
    gameListView->setRootIsDecorated(true);
    // game created width
    gameListView->setColumnWidth(1, gameListView->columnWidth(2) * 0.7);
    // players width
    gameListView->resizeColumnToContents(6);
    // description width
    gameListView->setColumnWidth(2, gameListView->columnWidth(2) * 1.7);
    // creator width
    gameListView->setColumnWidth(3, gameListView->columnWidth(3) * 1.2);
    // game type width
    gameListView->setColumnWidth(4, gameListView->columnWidth(4) * 1.4);
    if (room) {
        gameTypeMap = gameListModel->getGameTypes().value(room->getRoomId());
    }

    if (showFilters && restoresettings) {
        gameListProxyModel->loadFilterParameters(gameTypeMap);
    }

    auto *header = gameListView->header();
    header->setSectionsMovable(true);
    header->setFirstSectionMovable(true);
    header->setStretchLastSection(false);
    header->setSectionResizeMode(QHeaderView::Interactive);
    if (room) {
        header->hideSection(gameListModel->roomColIndex());
    }
    defaultHeaderState = header->saveState();
    const QByteArray savedState = SettingsCache::instance().layouts().getGameSelectorHeaderState(room != nullptr);
    if (!savedState.isEmpty() &&
        (!header->restoreState(savedState) || header->count() != gameListModel->columnCount() ||
         header->sortIndicatorSection() < 0 || header->sortIndicatorSection() >= gameListModel->columnCount())) {
        header->restoreState(defaultHeaderState);
    }
    // Visibility and resize policy belong to this view, not to a saved layout.
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(false);
    for (int column = 0; column < gameListModel->columnCount(); ++column) {
        header->showSection(column);
    }
    // Reapply hiding after sizing so Qt does not retain a hidden column's old width.
    if (room) {
        header->hideSection(gameListModel->roomColIndex());
    }
    gameListView->sortByColumn(header->sortIndicatorSection(), header->sortIndicatorOrder());

    headerSaveDebounceTimer.setSingleShot(true);
    headerSaveDebounceTimer.setInterval(500);
    connect(&headerSaveDebounceTimer, &QTimer::timeout, this, &GameSelector::saveHeaderState);
    // A newer edit or reset in another window supersedes our older pending save.
    connect(&SettingsCache::instance().layouts(), &LayoutsSettings::gameSelectorHeaderStateChangeStarted, this,
            [this](bool roomGames) {
                if (roomGames == (room != nullptr)) {
                    headerSaveDebounceTimer.stop();
                }
            });
    const auto headerChanged = [this] {
        SettingsCache::instance().layouts().beginGameSelectorHeaderStateChange(room != nullptr);
        headerSaveDebounceTimer.start();
    };
    connect(header, &QHeaderView::sectionMoved, this, headerChanged);
    connect(header, &QHeaderView::sectionResized, this, headerChanged);
    connect(header, &QHeaderView::sortIndicatorChanged, this, headerChanged);
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, &QHeaderView::customContextMenuRequested, this, [this, header](const QPoint &point) {
        auto *menu = new QMenu(header);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addAction(tr("Reset columns"), this, &GameSelector::resetHeaderState);
        menu->popup(header->mapToGlobal(point));
    });

    if (showFilters && restoresettings) {
        quickFilterToolBar = new GameSelectorQuickFilterToolBar(this, tabSupervisor, gameListProxyModel, gameTypeMap);
        quickFilterToolBar->setVisible(showFilters && restoresettings &&
                                       SettingsCache::instance().userInterface().getShowGameSelectorFilterToolbar());

        connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::showGameSelectorFilterToolbarChanged,
                this, [this] {
                    quickFilterToolBar->setVisible(
                        SettingsCache::instance().userInterface().getShowGameSelectorFilterToolbar());
                });
        // The quick-filter toolbar only updates the in-memory model, so persist every change.
        // Otherwise each new GameSelector (room switch, reconnect, restart) would revert to stale
        // saved values and e.g. hide full/started games the user had unchecked. The save is
        // debounced so typing in the search field does not rewrite the settings file on every
        // keystroke; a pending save is flushed in the destructor.
        filterSaveDebounceTimer.setSingleShot(true);
        filterSaveDebounceTimer.setInterval(500);
        connect(&filterSaveDebounceTimer, &QTimer::timeout, this,
                [this] { gameListProxyModel->saveFilterParameters(gameTypeMap); });
        connect(gameListProxyModel, &GamesProxyModel::filtersChanged, this,
                [this] { filterSaveDebounceTimer.start(); });
    } else {
        quickFilterToolBar = nullptr;
    }

    filterButton = new QPushButton;
    filterButton->setIcon(themePixmap(QStringLiteral("icons/search")));
    connect(filterButton, &QPushButton::clicked, this, &GameSelector::actSetFilter);
    clearFilterButton = new QPushButton;
    clearFilterButton->setIcon(themePixmap(QStringLiteral("icons/clearsearch")));
    bool filtersSetToDefault = showFilters && gameListProxyModel->areFilterParametersSetToDefaults();
    clearFilterButton->setEnabled(!filtersSetToDefault);
    connect(clearFilterButton, &QPushButton::clicked, this, &GameSelector::actClearFilter);
    connect(gameListProxyModel, &GamesProxyModel::filtersChanged, this, &GameSelector::checkClearFilterButtonState);

    if (room) {
        createButton = new QPushButton;
        connect(createButton, &QPushButton::clicked, this, &GameSelector::actCreate);
    } else {
        createButton = nullptr;
    }
    joinButton = new QPushButton;
    joinAsJudgeButton = new QPushButton;
    spectateButton = new QPushButton;
    joinAsJudgeSpectatorButton = new QPushButton;

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    if (showFilters) {
        buttonLayout->addWidget(filterButton);
        buttonLayout->addWidget(clearFilterButton);
    }
    buttonLayout->addStretch();
    if (room) {
        buttonLayout->addWidget(createButton);
    }
    buttonLayout->addWidget(joinButton);
    if (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsJudge) {
        buttonLayout->addWidget(joinAsJudgeButton);
    } else {
        joinAsJudgeButton->setHidden(true);
    }
    buttonLayout->addWidget(spectateButton);
    if (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsJudge) {
        buttonLayout->addWidget(joinAsJudgeSpectatorButton);
    } else {
        joinAsJudgeSpectatorButton->setHidden(true);
    }
    buttonLayout->setAlignment(Qt::AlignTop);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    if (showFilters && restoresettings) {
        mainLayout->addWidget(quickFilterToolBar);
    }
    mainLayout->addWidget(gameListView);
    mainLayout->addLayout(buttonLayout);

    retranslateUi();
    setLayout(mainLayout);

    setMinimumWidth((qreal)(gameListView->columnWidth(0) * gameListModel->columnCount()) / 1.5);
    setMinimumHeight(200);

    connect(joinButton, &QPushButton::clicked, this, &GameSelector::actJoin);
    connect(joinAsJudgeButton, &QPushButton::clicked, this, &GameSelector::actJoinAsJudge);
    connect(spectateButton, &QPushButton::clicked, this, &GameSelector::actJoinAsSpectator);
    connect(joinAsJudgeSpectatorButton, &QPushButton::clicked, this, &GameSelector::actJoinAsJudgeSpectator);
    connect(gameListView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &GameSelector::actSelectedGameChanged);
    connect(gameListView, &QTreeView::activated, this, &GameSelector::actJoin);

    connect(client, &AbstractClient::ignoreListReceived, this, &GameSelector::ignoreListReceived);
    connect(client, &AbstractClient::addToListEventReceived, this, &GameSelector::processAddToListEvent);
    connect(client, &AbstractClient::removeFromListEventReceived, this, &GameSelector::processRemoveFromListEvent);
}

GameSelector::~GameSelector()
{
    // Only flush pending edits; an untouched older tab must not overwrite a newer layout.
    if (headerSaveDebounceTimer.isActive()) {
        saveHeaderState();
    }
    // Nothing else persists quick-filter toolbar edits: the toolbar only updates the in-memory model,
    // and the debounced timer above is the sole writer for that path (actSetFilter/actClearFilter save
    // synchronously, but the toolbar does not). Flush a pending debounced save so edits made within
    // the debounce interval before destruction are not lost on room switch, reconnect, or close.
    if (filterSaveDebounceTimer.isActive()) {
        filterSaveDebounceTimer.stop();
        gameListProxyModel->saveFilterParameters(gameTypeMap);
    }
}

void GameSelector::saveHeaderState()
{
    headerSaveDebounceTimer.stop();
    SettingsCache::instance().layouts().setGameSelectorHeaderState(gameListView->header()->saveState(),
                                                                   room != nullptr);
}

void GameSelector::resetHeaderState()
{
    headerSaveDebounceTimer.stop();
    auto *header = gameListView->header();
    const QSignalBlocker blocker(header);
    header->restoreState(defaultHeaderState);
    gameListView->sortByColumn(header->sortIndicatorSection(), header->sortIndicatorOrder());
    SettingsCache::instance().layouts().setGameSelectorHeaderState({}, room != nullptr);
}

void GameSelector::ignoreListReceived(const QList<ServerInfo_User> &)
{
    gameListProxyModel->refresh();
}

void GameSelector::processAddToListEvent(const Event_AddToList &event)
{
    if (event.list_name() == "ignore") {
        gameListProxyModel->refresh();
    }
    updateTitle();
}

void GameSelector::processRemoveFromListEvent(const Event_RemoveFromList &event)
{
    if (event.list_name() == "ignore") {
        gameListProxyModel->refresh();
    }
    updateTitle();
}

void GameSelector::actSetFilter()
{
    DlgFilterGames dlg(gameTypeMap, gameListProxyModel, this);

    if (!dlg.exec()) {
        return;
    }

    gameListProxyModel->setGameFilters(dlg.getFilters());
    gameListProxyModel->saveFilterParameters(gameTypeMap);

    updateTitle();
}

void GameSelector::checkClearFilterButtonState()
{
    clearFilterButton->setEnabled(!gameListProxyModel->areFilterParametersSetToDefaults());
}

void GameSelector::actClearFilter()
{
    gameListProxyModel->resetFilterParameters();
    gameListProxyModel->saveFilterParameters(gameTypeMap);

    updateTitle();
}

void GameSelector::actCreate()
{
    if (room == nullptr) {
        qWarning() << "Attempted to create game, but the room was null";
        return;
    }

    DlgCreateGame dlg(room, room->getGameTypes(), this);
    dlg.exec();
    updateTitle();
}

void GameSelector::checkResponse(const Response &response)
{
    // NB: We re-enable buttons for the currently selected game, which may not
    // be the same game as the one for which we are processing a response.
    // This could lead to situations where we join a game, select a different
    // game, join the new game, then receive the response for the original
    // join, which would re-activate the buttons for the second game too soon.
    // However, that is better than doing things the other ways, because then
    // the response to the first game could lock us out of join/join as
    // spectator for the second game!
    //
    // Ideally we should have a way to figure out if the current game is the
    // same as the one we are getting a response for, but it's probably not
    // worth the trouble.
    enableButtons();

    switch (response.response_code()) {
        case Response::RespNotInRoom:
            QMessageBox::critical(this, tr("Error"), tr("Please join the appropriate room first."));
            break;
        case Response::RespWrongPassword:
            QMessageBox::critical(this, tr("Error"), tr("Wrong password."));
            break;
        case Response::RespSpectatorsNotAllowed:
            QMessageBox::critical(this, tr("Error"), tr("Spectators are not allowed in this game."));
            break;
        case Response::RespGameFull:
            QMessageBox::critical(this, tr("Error"), tr("The game is already full."));
            break;
        case Response::RespNameNotFound:
            QMessageBox::critical(this, tr("Error"), tr("The game does not exist any more."));
            break;
        case Response::RespUserLevelTooLow:
            QMessageBox::critical(this, tr("Error"), tr("This game is only open to registered users."));
            break;
        case Response::RespOnlyBuddies:
            QMessageBox::critical(this, tr("Error"), tr("This game is only open to its creator's buddies."));
            break;
        case Response::RespInIgnoreList:
            QMessageBox::critical(this, tr("Error"), tr("You are being ignored by the creator of this game."));
            break;
        default:;
    }
}

void GameSelector::actJoin()
{
    joinGame();
}

void GameSelector::actJoinAsJudge()
{
    if (!(tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsJudge)) {
        joinGame();
    } else {
        joinGame(false, true);
    }
}

void GameSelector::actJoinAsSpectator()
{
    joinGame(true);
}

void GameSelector::actJoinAsJudgeSpectator()
{
    if (!(tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsJudge)) {
        joinGame(true);
    } else {
        joinGame(true, true);
    }
}

void GameSelector::customContextMenu(const QPoint &point)
{
    // Sorting and server updates can move or remove rows while the menu is open.
    const QPersistentModelIndex index(gameListView->indexAt(point));
    if (!index.isValid()) {
        return;
    }
    const auto selectedGame = [this, index]() -> std::optional<ServerInfo_Game> {
        if (!index.isValid()) {
            return std::nullopt;
        }
        // Keep a value across join confirmations, which can process more server updates.
        return gameListModel->getGame(index.data(Qt::UserRole).toInt());
    };

    auto *menu = new QMenu(gameListView);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(tr("Join Game"), this, [this, selectedGame] {
        if (const auto game = selectedGame()) {
            joinGame(*game);
        }
    });
    if (tabSupervisor->getUserInfo()->user_level() & ServerInfo_User::IsJudge) {
        menu->addAction(tr("Join Game as Judge"), this, [this, selectedGame] {
            if (const auto game = selectedGame()) {
                joinGame(*game, false, true);
            }
        });
        menu->addAction(tr("Spectate Game as Judge"), this, [this, selectedGame] {
            if (const auto game = selectedGame()) {
                joinGame(*game, true, true);
            }
        });
    }
    menu->addAction(tr("Spectate Game"), this, [this, selectedGame] {
        if (const auto game = selectedGame()) {
            joinGame(*game, true);
        }
    });
    menu->addAction(tr("Game Information"), this, [this, selectedGame] {
        if (const auto game = selectedGame()) {
            const auto gameTypes = gameListModel->getGameTypes().value(game->room_id());
            auto *dialog = new DlgCreateGame(*game, gameTypes, this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->open();
        }
    });
    if (!client->serverName().isEmpty()) {
        menu->addAction(tr("Cop&y game link"), this, [this, selectedGame] {
            if (const auto game = selectedGame()) {
                QGuiApplication::clipboard()->setText(makeGameJoinLink(client->serverName(), client->serverPort(),
                                                                       game->room_id(), game->game_id(),
                                                                       QString::fromStdString(game->description())));
            }
        });
    }
    menu->popup(gameListView->viewport()->mapToGlobal(point));
}

void GameSelector::joinGame(const bool asSpectator, const bool asJudge)
{
    QModelIndex ind = gameListView->currentIndex();
    if (!ind.isValid()) {
        return;
    }

    joinGame(gameListModel->getGame(ind.data(Qt::UserRole).toInt()), asSpectator, asJudge);
}

void GameSelector::joinGame(const ServerInfo_Game &game, const bool asSpectator, const bool asJudge)
{
    if (tabSupervisor->switchToGameTabIfAlreadyExists(game.game_id())) {
        return;
    }

    bool overrideRestrictions = tabSupervisor->canOverrideGameRestrictions();

    // Joining a full game without override privileges silently becomes a
    // spectator join, so ask first instead of surprising the player.
    const bool gameFull = game.player_count() == game.max_players();
    if (gameFull && !asSpectator && !asJudge && !overrideRestrictions) {
        if (!QtUtils::confirm(this, tr("Join game"), tr("The game is full. Join as a spectator instead?"),
                              QMessageBox::No)) {
            return;
        }
    }

    bool spectator = asSpectator || gameFull;

    QString password;
    if (game.with_password() && !(spectator && !game.spectators_need_password()) && !overrideRestrictions) {
        bool ok;
        // Games without a description have no sensible label — fall back to the
        // game id so the prompt still tells the user which game they're entering.
        const QString gameLabel = QString::fromStdString(game.description());
        const QString prompt = gameLabel.isEmpty() ? tr("Password for game #%1:").arg(game.game_id())
                                                   : tr("Password for \"%1\":").arg(gameLabel);
        password = getTextWithMax(this, tr("Join game"), prompt, QLineEdit::Password, QString(), &ok);
        if (!ok) {
            return;
        }
    }

    Command_JoinGame cmd;
    cmd.set_game_id(game.game_id());
    cmd.set_password(password.toStdString());
    cmd.set_spectator(spectator);
    cmd.set_override_restrictions(overrideRestrictions);
    cmd.set_join_as_judge(asJudge);

    TabRoom *r = tabSupervisor->getRoomTabs().value(game.room_id());
    if (!r) {
        QMessageBox::critical(this, tr("Error"), tr("Please join the respective room first."));
        return;
    }

    PendingCommand *pend = r->prepareRoomCommand(cmd);
    connect(pend, &PendingCommand::finished, this, &GameSelector::checkResponse);
    r->sendRoomCommand(pend);

    disableButtons();
}

bool GameSelector::joinGameById(const int gameId, const bool asSpectator)
{
    for (int row = 0; row < gameListModel->rowCount(); ++row) {
        const ServerInfo_Game &game = gameListModel->getGame(row);
        if (game.game_id() != gameId) {
            continue;
        }

        joinGame(game, asSpectator);
        return true;
    }

    qWarning() << "Game" << gameId << "not found";
    return false;
}

void GameSelector::disableButtons()
{
    if (createButton) {
        createButton->setEnabled(false);
    }

    joinButton->setEnabled(false);
    spectateButton->setEnabled(false);
}

void GameSelector::enableButtons()
{
    if (createButton) {
        createButton->setEnabled(true);
    }

    // Enable buttons for the currently selected game
    enableButtonsForIndex(gameListView->currentIndex());
}

void GameSelector::enableButtonsForIndex(const QModelIndex &current)
{
    if (!current.isValid()) {
        return;
    }

    const ServerInfo_Game &game = gameListModel->getGame(current.data(Qt::UserRole).toInt());
    bool overrideRestrictions = tabSupervisor->canOverrideGameRestrictions();

    spectateButton->setEnabled(game.spectators_allowed() || overrideRestrictions);
    joinButton->setEnabled(game.player_count() < game.max_players() || overrideRestrictions);
}

void GameSelector::retranslateUi()
{
    filterButton->setText(tr("&Filter games"));
    clearFilterButton->setText(tr("C&lear filter"));
    if (createButton) {
        createButton->setText(tr("C&reate"));
    }
    joinButton->setText(tr("&Join"));
    joinAsJudgeButton->setText(tr("Join as judge"));
    spectateButton->setText(tr("J&oin as spectator"));
    joinAsJudgeSpectatorButton->setText(tr("Join as judge spectator"));

    updateTitle();
}

void GameSelector::processGameInfo(const ServerInfo_Game &info)
{
    gameListModel->updateGameList(info);
    updateTitle();
}

void GameSelector::actSelectedGameChanged(const QModelIndex &current, const QModelIndex & /* previous */)
{
    enableButtonsForIndex(current);
}

void GameSelector::updateTitle()
{
    if (showFilters) {
        const int totalGames = gameListModel->rowCount();
        const int shownGames = totalGames - gameListProxyModel->getNumFilteredGames();
        setTitle(tr("Games shown: %1 / %2").arg(shownGames).arg(totalGames));
    } else {
        setTitle(tr("Games"));
    }
}
