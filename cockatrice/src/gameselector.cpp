#include <QTreeView>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QInputDialog>
#include "tab_supervisor.h"
#include "dlg_creategame.h"
#include "dlg_filter_games.h"
#include "gameselector.h"
#include "gamesmodel.h"
#include "tab_room.h"
#include "pending_command.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "pb/response.pb.h"

GameSelector::GameSelector(AbstractClient *_client, const TabSupervisor *_tabSupervisor, TabRoom *_room, const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QWidget *parent)
    : QGroupBox(parent), client(_client), tabSupervisor(_tabSupervisor), room(_room)
{
    gameListView = new QTreeView;
    gameListModel = new GamesModel(_rooms, _gameTypes, this);
    gameListProxyModel = new GamesProxyModel(this, tabSupervisor->getUserInfo());
    gameListProxyModel->setSourceModel(gameListModel);
    gameListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    gameListView->setModel(gameListProxyModel);
    gameListView->setSortingEnabled(true);
    gameListView->setAlternatingRowColors(true);
    gameListView->setRootIsDecorated(true);
    if (_room)
        gameListView->header()->hideSection(1);
    else
        gameListProxyModel->setUnavailableGamesVisible(true);
    gameListView->header()->setResizeMode(1, QHeaderView::ResizeToContents);

    filterButton = new QPushButton;
    filterButton->setIcon(QIcon(":/resources/icon_search.svg"));
    connect(filterButton, SIGNAL(clicked()), this, SLOT(actSetFilter()));
    clearFilterButton = new QPushButton;
    clearFilterButton->setIcon(QIcon(":/resources/icon_clearsearch.svg"));
    clearFilterButton->setEnabled(false);
    connect(clearFilterButton, SIGNAL(clicked()), this, SLOT(actClearFilter()));

    if (room) {
        createButton = new QPushButton;
        connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
    } else
        createButton = 0;
    joinButton = new QPushButton;
    spectateButton = new QPushButton;
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(filterButton);
    buttonLayout->addWidget(clearFilterButton);
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

    setMinimumWidth((qreal) (gameListView->columnWidth(0) * gameListModel->columnCount()) / 1.5);
    setMinimumHeight(200);

    connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
    connect(spectateButton, SIGNAL(clicked()), this, SLOT(actJoin()));
}

void GameSelector::actSetFilter()
{
    GameTypeMap gameTypeMap;
    if (room)
        gameTypeMap = gameListModel->getGameTypes().value(room->getRoomId());
    DlgFilterGames dlg(gameTypeMap, this);
    dlg.setUnavailableGamesVisible(gameListProxyModel->getUnavailableGamesVisible());
    dlg.setPasswordProtectedGamesVisible(gameListProxyModel->getPasswordProtectedGamesVisible());
    dlg.setGameNameFilter(gameListProxyModel->getGameNameFilter());
    dlg.setCreatorNameFilter(gameListProxyModel->getCreatorNameFilter());
    dlg.setGameTypeFilter(gameListProxyModel->getGameTypeFilter());
    dlg.setMaxPlayersFilter(gameListProxyModel->getMaxPlayersFilterMin(), gameListProxyModel->getMaxPlayersFilterMax());
    
    if (!dlg.exec())
        return;
    
    clearFilterButton->setEnabled(true);
    
    gameListProxyModel->setUnavailableGamesVisible(dlg.getUnavailableGamesVisible());
    gameListProxyModel->setPasswordProtectedGamesVisible(dlg.getPasswordProtectedGamesVisible());
    gameListProxyModel->setGameNameFilter(dlg.getGameNameFilter());
    gameListProxyModel->setCreatorNameFilter(dlg.getCreatorNameFilter());
    gameListProxyModel->setGameTypeFilter(dlg.getGameTypeFilter());
    gameListProxyModel->setMaxPlayersFilter(dlg.getMaxPlayersFilterMin(), dlg.getMaxPlayersFilterMax());
}

void GameSelector::actClearFilter()
{
    clearFilterButton->setEnabled(false);
    
    gameListProxyModel->resetFilterParameters();
}

void GameSelector::actCreate()
{
    DlgCreateGame dlg(room, room->getGameTypes(), this);
    dlg.exec();
}

void GameSelector::checkResponse(const Response &response)
{
    if (createButton)
        createButton->setEnabled(true);
    joinButton->setEnabled(true);
    spectateButton->setEnabled(true);

    switch (response.response_code()) {
        case Response::RespNotInRoom: QMessageBox::critical(this, tr("Error"), tr("Please join the appropriate room first.")); break;
        case Response::RespWrongPassword: QMessageBox::critical(this, tr("Error"), tr("Wrong password.")); break;
        case Response::RespSpectatorsNotAllowed: QMessageBox::critical(this, tr("Error"), tr("Spectators are not allowed in this game.")); break;
        case Response::RespGameFull: QMessageBox::critical(this, tr("Error"), tr("The game is already full.")); break;
        case Response::RespNameNotFound: QMessageBox::critical(this, tr("Error"), tr("The game does not exist any more.")); break;
        case Response::RespUserLevelTooLow: QMessageBox::critical(this, tr("Error"), tr("This game is only open to registered users.")); break;
        case Response::RespOnlyBuddies: QMessageBox::critical(this, tr("Error"), tr("This game is only open to its creator's buddies.")); break;
        case Response::RespInIgnoreList: QMessageBox::critical(this, tr("Error"), tr("You are being ignored by the creator of this game.")); break;
        default: ;
    }
}

void GameSelector::actJoin()
{
    bool spectator = sender() == spectateButton;
    
    QModelIndex ind = gameListView->currentIndex();
    if (!ind.isValid())
        return;
    const ServerInfo_Game &game = gameListModel->getGame(ind.data(Qt::UserRole).toInt());
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
    setTitle(tr("Games"));
    filterButton->setText(tr("&Filter games"));
    clearFilterButton->setText(tr("C&lear filter"));
    if (createButton)
        createButton->setText(tr("C&reate"));
    joinButton->setText(tr("&Join"));
    spectateButton->setText(tr("J&oin as spectator"));
}

void GameSelector::processGameInfo(const ServerInfo_Game &info)
{
    gameListModel->updateGameList(info);
}
