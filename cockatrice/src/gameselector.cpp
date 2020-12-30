#include "gameselector.h"

#include "abstractclient.h"
#include "dlg_creategame.h"
#include "dlg_filter_games.h"
#include "gamesmodel.h"
#include "pb/response.pb.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "pending_command.h"
#include "tab_account.h"
#include "tab_room.h"
#include "tab_supervisor.h"

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
                           const TabSupervisor *_tabSupervisor,
                           TabRoom *_room,
                           const QMap<int, QString> &_rooms,
                           const QMap<int, GameTypeMap> &_gameTypes,
                           const bool restoresettings,
                           const bool _showfilters,
                           QWidget *parent)
    : QGroupBox(parent), client(_client), tabSupervisor(_tabSupervisor), room(_room), showFilters(_showfilters)
{
    gameListView = new QTreeView;
    gameListModel = new GamesModel(_rooms, _gameTypes, this);
    if (showFilters) {
        gameListProxyModel = new GamesProxyModel(this, tabSupervisor);
        gameListProxyModel->setSourceModel(gameListModel);
        gameListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        gameListView->setModel(gameListProxyModel);
    } else {
        gameListView->setModel(gameListModel);
    }
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
    connect(filterButton, SIGNAL(clicked()), this, SLOT(actSetFilter()));
    clearFilterButton = new QPushButton;
    clearFilterButton->setIcon(QPixmap("theme:icons/clearsearch"));
    bool filtersSetToDefault = showFilters && gameListProxyModel->areFilterParametersSetToDefaults();
    clearFilterButton->setEnabled(!filtersSetToDefault);
    connect(clearFilterButton, SIGNAL(clicked()), this, SLOT(actClearFilter()));

    if (room) {
        createButton = new QPushButton;
        connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
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

    connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
    connect(spectateButton, SIGNAL(clicked()), this, SLOT(actJoin()));
    connect(gameListView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this,
            SLOT(actSelectedGameChanged(const QModelIndex &, const QModelIndex &)));
    connect(gameListView, SIGNAL(activated(const QModelIndex &)), this, SLOT(actJoin()));

    connect(client, SIGNAL(ignoreListReceived(const QList<ServerInfo_User> &)), this,
            SLOT(ignoreListReceived(const QList<ServerInfo_User> &)));
    connect(client, SIGNAL(addToListEventReceived(const Event_AddToList &)), this,
            SLOT(processAddToListEvent(const Event_AddToList &)));
    connect(client, SIGNAL(removeFromListEventReceived(const Event_RemoveFromList &)), this,
            SLOT(processRemoveFromListEvent(const Event_RemoveFromList &)));
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

    gameListProxyModel->setShowBuddiesOnlyGames(dlg.getShowBuddiesOnlyGames());
    gameListProxyModel->setShowFullGames(dlg.getShowFullGames());
    gameListProxyModel->setShowGamesThatStarted(dlg.getShowGamesThatStarted());
    gameListProxyModel->setShowPasswordProtectedGames(dlg.getShowPasswordProtectedGames());
    gameListProxyModel->setHideIgnoredUserGames(dlg.getHideIgnoredUserGames());
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
    if (createButton)
        createButton->setEnabled(true);
    joinButton->setEnabled(true);

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

    if (response.response_code() != Response::RespSpectatorsNotAllowed)
        spectateButton->setEnabled(true);
}

void GameSelector::actJoin()
{
    QModelIndex ind = gameListView->currentIndex();
    if (!ind.isValid())
        return;
    const ServerInfo_Game &game = gameListModel->getGame(ind.data(Qt::UserRole).toInt());
    bool spectator = sender() == spectateButton || game.player_count() == game.max_players();
    bool overrideRestrictions = !tabSupervisor->getAdminLocked();
    QString password;
    if (game.with_password() && !(spectator && !game.spectators_need_password()) && !overrideRestrictions) {
        bool ok;
        password = QInputDialog::getText(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
        if (!ok)
            return;
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
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(checkResponse(Response)));
    r->sendRoomCommand(pend);

    if (createButton)
        createButton->setEnabled(false);
    joinButton->setEnabled(false);
    spectateButton->setEnabled(false);
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
    if (!current.isValid())
        return;

    const ServerInfo_Game &game = gameListModel->getGame(current.data(Qt::UserRole).toInt());
    bool overrideRestrictions = !tabSupervisor->getAdminLocked();

    spectateButton->setEnabled(game.spectators_allowed() || overrideRestrictions);
    joinButton->setEnabled(game.player_count() < game.max_players() || overrideRestrictions);
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
