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
#include "gameselector.h"
#include "gamesmodel.h"
#include "tab_room.h"
#include "pending_command.h"
#include "pb/room_commands.pb.h"
#include "pb/serverinfo_game.pb.h"

GameSelector::GameSelector(AbstractClient *_client, TabSupervisor *_tabSupervisor, TabRoom *_room, const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QWidget *parent)
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
	gameListView->header()->setResizeMode(1, QHeaderView::ResizeToContents);

	showUnavailableGamesCheckBox = new QCheckBox;
	
	QVBoxLayout *filterLayout = new QVBoxLayout;
	filterLayout->addWidget(showUnavailableGamesCheckBox);
	
	if (room) {
		createButton = new QPushButton;
		connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	} else
		createButton = 0;
	joinButton = new QPushButton;
	spectateButton = new QPushButton;
	
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	if (room)
		buttonLayout->addWidget(createButton);
	buttonLayout->addWidget(joinButton);
	buttonLayout->addWidget(spectateButton);
	buttonLayout->setAlignment(Qt::AlignTop);
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(filterLayout);
	hbox->addStretch();
	hbox->addLayout(buttonLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gameListView);
	mainLayout->addLayout(hbox);

	retranslateUi();
	setLayout(mainLayout);

	setMinimumWidth((qreal) (gameListView->columnWidth(0) * gameListModel->columnCount()) / 1.5);
	setMinimumHeight(200);

	connect(showUnavailableGamesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showUnavailableGamesChanged(int)));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
	connect(spectateButton, SIGNAL(clicked()), this, SLOT(actJoin()));
}

void GameSelector::showUnavailableGamesChanged(int state)
{
	gameListProxyModel->setUnavailableGamesVisible(state);
}

void GameSelector::actCreate()
{
	DlgCreateGame dlg(room, room->getGameTypes(), this);
	dlg.exec();
}

void GameSelector::checkResponse(Response::ResponseCode response)
{
	if (createButton)
		createButton->setEnabled(true);
	joinButton->setEnabled(true);
	spectateButton->setEnabled(true);

	switch (response) {
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
	connect(pend, SIGNAL(finished(Response::ResponseCode)), this, SLOT(checkResponse(Response::ResponseCode)));
	r->sendRoomCommand(pend);

	if (createButton)
		createButton->setEnabled(false);
	joinButton->setEnabled(false);
	spectateButton->setEnabled(false);
}

void GameSelector::retranslateUi()
{
	setTitle(tr("Games"));
	showUnavailableGamesCheckBox->setText(tr("Show u&navailable games"));
	if (createButton)
		createButton->setText(tr("C&reate"));
	joinButton->setText(tr("&Join"));
	spectateButton->setText(tr("J&oin as spectator"));
}

void GameSelector::processGameInfo(const ServerInfo_Game &info)
{
	gameListModel->updateGameList(info);
}
