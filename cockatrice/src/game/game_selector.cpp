#include "game_selector.h"

#include "../client/get_text_with_max.h"
#include "../client/tabs/tab_account.h"
#include "../client/tabs/tab_game.h"
#include "../client/tabs/tab_room.h"
#include "../client/tabs/tab_supervisor.h"
#include "../dialogs/dlg_create_game.h"
#include "../dialogs/dlg_filter_games.h"
#include "../server/abstract_client.h"
#include "../server/pending_command.h"
#include "../server/user/user_list_manager.h"
#include "games_model.h"
#include "pb/response.pb.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_game.pb.h"

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

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
    if (showFilters) {
        gameListProxyModel = new GamesProxyModel(this, tabSupervisor->getUserListManager());
        gameListProxyModel->setSourceModel(gameListModel);
        gameListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        gameListView->setModel(gameListProxyModel);
    } else {
        gameListView->setModel(gameListModel);
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
    if (_room)
        gameListView->header()->hideSection(gameListModel->roomColIndex());

    if (room)
        gameTypeMap = gameListModel->getGameTypes().value(room->getRoomId());

    if (showFilters && restoresettings)
        gameListProxyModel->loadFilterParameters(gameTypeMap);

    gameListView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    filterButton = new QPushButton;
    filterButton->setIcon(QPixmap("theme:icons/search"));
    connect(filterButton, &QPushButton::clicked, this, &GameSelector::actSetFilter);
    clearFilterButton = new QPushButton;
    clearFilterButton->setIcon(QPixmap("theme:icons/clearsearch"));
    bool filtersSetToDefault = showFilters && gameListProxyModel->areFilterParametersSetToDefaults();
    clearFilterButton->setEnabled(!filtersSetToDefault);
    connect(clearFilterButton, &QPushButton::clicked, this, &GameSelector::actClearFilter);

    if (room) {
        createButton = new QPushButton;
        connect(createButton, &QPushButton::clicked, this, &GameSelector::actCreate);
    } else {
        createButton = nullptr;
    }
    joinButton = new QPushButton;
    spectateButton = new QPushButton;

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    if (showFilters) {
        buttonLayout->addWidget(filterButton);
        buttonLayout->addWidget(clearFilterButton);
    }
    buttonLayout->addStretch();
    if (room)
        buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(joinButton);
    buttonLayout->addWidget(spectateButton);
    buttonLayout->setAlignment(Qt::AlignTop);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gameListView);
    mainLayout->addLayout(buttonLayout);

    retranslateUi();
    setLayout(mainLayout);

    setMinimumWidth((qreal)(gameListView->columnWidth(0) * gameListModel->columnCount()) / 1.5);
    setMinimumHeight(200);

    connect(joinButton, &QPushButton::clicked, this, &GameSelector::actJoin);
    connect(spectateButton, &QPushButton::clicked, this, &GameSelector::actSpectate);
    connect(gameListView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &GameSelector::actSelectedGameChanged);
    connect(gameListView, &QTreeView::activated, this, &GameSelector::actJoin);

    connect(client, &AbstractClient::ignoreListReceived, this, &GameSelector::ignoreListReceived);
    connect(client, &AbstractClient::addToListEventReceived, this, &GameSelector::processAddToListEvent);
    connect(client, &AbstractClient::removeFromListEventReceived, this, &GameSelector::processRemoveFromListEvent);
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

    if (!dlg.exec())
        return;

    gameListProxyModel->setHideBuddiesOnlyGames(dlg.getHideBuddiesOnlyGames());
    gameListProxyModel->setHideFullGames(dlg.getHideFullGames());
    gameListProxyModel->setHideGamesThatStarted(dlg.getHideGamesThatStarted());
    gameListProxyModel->setHidePasswordProtectedGames(dlg.getHidePasswordProtectedGames());
    gameListProxyModel->setHideIgnoredUserGames(dlg.getHideIgnoredUserGames());
    gameListProxyModel->setHideNotBuddyCreatedGames(dlg.getHideNotBuddyCreatedGames());
    gameListProxyModel->setGameNameFilter(dlg.getGameNameFilter());
    gameListProxyModel->setCreatorNameFilter(dlg.getCreatorNameFilter());
    gameListProxyModel->setGameTypeFilter(dlg.getGameTypeFilter());
    gameListProxyModel->setMaxPlayersFilter(dlg.getMaxPlayersFilterMin(), dlg.getMaxPlayersFilterMax());
    gameListProxyModel->setMaxGameAge(dlg.getMaxGameAge());
    gameListProxyModel->setShowOnlyIfSpectatorsCanWatch(dlg.getShowOnlyIfSpectatorsCanWatch());
    gameListProxyModel->setShowSpectatorPasswordProtected(dlg.getShowSpectatorPasswordProtected());
    gameListProxyModel->setShowOnlyIfSpectatorsCanChat(dlg.getShowOnlyIfSpectatorsCanChat());
    gameListProxyModel->setShowOnlyIfSpectatorsCanSeeHands(dlg.getShowOnlyIfSpectatorsCanSeeHands());
    gameListProxyModel->saveFilterParameters(gameTypeMap);

    clearFilterButton->setEnabled(!gameListProxyModel->areFilterParametersSetToDefaults());

    updateTitle();
}

void GameSelector::actClearFilter()
{
    clearFilterButton->setEnabled(false);

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
    return joinGame(false);
}

void GameSelector::actSpectate()
{
    return joinGame(true);
}

void GameSelector::customContextMenu(const QPoint &point)
{
    const auto &index = gameListView->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QAction joinGame(tr("Join Game"));
    connect(&joinGame, &QAction::triggered, this, &GameSelector::actJoin);

    QAction spectateGame(tr("Spectate Game"));
    connect(&spectateGame, &QAction::triggered, this, &GameSelector::actSpectate);

    QAction getGameInfo(tr("Game Information"));
    connect(&getGameInfo, &QAction::triggered, this, [=, this]() {
        const ServerInfo_Game &gameInfo = gameListModel->getGame(index.data(Qt::UserRole).toInt());
        const QMap<int, QString> &gameTypes = gameListModel->getGameTypes().value(gameInfo.room_id());

        DlgCreateGame dlg(gameInfo, gameTypes, this);
        dlg.exec();
    });

    QMenu menu;
    menu.addAction(&joinGame);
    menu.addAction(&spectateGame);
    menu.addAction(&getGameInfo);
    menu.exec(gameListView->mapToGlobal(point));
}

void GameSelector::joinGame(const bool isSpectator)
{
    QModelIndex ind = gameListView->currentIndex();
    if (!ind.isValid()) {
        return;
    }

    const ServerInfo_Game &game = gameListModel->getGame(ind.data(Qt::UserRole).toInt());
    if (tabSupervisor->switchToGameTabIfAlreadyExists(game.game_id())) {
        return;
    }

    bool spectator = isSpectator || game.player_count() == game.max_players();

    bool overrideRestrictions = !tabSupervisor->getAdminLocked();
    QString password;
    if (game.with_password() && !(spectator && !game.spectators_need_password()) && !overrideRestrictions) {
        bool ok;
        password = getTextWithMax(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
        if (!ok) {
            return;
        }
    }

    Command_JoinGame cmd;
    cmd.set_game_id(game.game_id());
    cmd.set_password(password.toStdString());
    cmd.set_spectator(spectator);
    cmd.set_override_restrictions(overrideRestrictions);
    cmd.set_join_as_judge((QApplication::keyboardModifiers() & Qt::ShiftModifier) != 0);

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

void GameSelector::disableButtons()
{
    if (createButton)
        createButton->setEnabled(false);

    joinButton->setEnabled(false);
    spectateButton->setEnabled(false);
}

void GameSelector::enableButtons()
{
    if (createButton)
        createButton->setEnabled(true);

    // Enable buttons for the currently selected game
    enableButtonsForIndex(gameListView->currentIndex());
}

void GameSelector::enableButtonsForIndex(const QModelIndex &current)
{
    if (!current.isValid())
        return;

    const ServerInfo_Game &game = gameListModel->getGame(current.data(Qt::UserRole).toInt());
    bool overrideRestrictions = !tabSupervisor->getAdminLocked();

    spectateButton->setEnabled(game.spectators_allowed() || overrideRestrictions);
    joinButton->setEnabled(game.player_count() < game.max_players() || overrideRestrictions);
}

void GameSelector::retranslateUi()
{
    filterButton->setText(tr("&Filter games"));
    clearFilterButton->setText(tr("C&lear filter"));
    if (createButton)
        createButton->setText(tr("C&reate"));
    joinButton->setText(tr("&Join"));
    spectateButton->setText(tr("J&oin as spectator"));

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
