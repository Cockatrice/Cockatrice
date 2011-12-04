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
#include "abstractclient.h"
#include "protocol_items.h"
#include "gameselector.h"
#include "gamesmodel.h"

GameSelector::GameSelector(AbstractClient *_client, TabSupervisor *_tabSupervisor, TabRoom *_room, const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QWidget *parent)
	: QGroupBox(parent), client(_client), tabSupervisor(_tabSupervisor), room(_room)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(_rooms, _gameTypes, this);
	gameListProxyModel = new GamesProxyModel(this);
	gameListProxyModel->setSourceModel(gameListModel);
	gameListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	gameListView->setModel(gameListProxyModel);
	gameListView->setSortingEnabled(true);
	gameListView->setAlternatingRowColors(true);
	gameListView->setRootIsDecorated(true);
	if (_room)
		gameListView->header()->hideSection(1);
	gameListView->header()->setResizeMode(1, QHeaderView::ResizeToContents);

	showUnjoinableGamesCheckBox = new QCheckBox;
	
	QVBoxLayout *filterLayout = new QVBoxLayout;
	filterLayout->addWidget(showUnjoinableGamesCheckBox);
	
	if (room)
		createButton = new QPushButton;
	else
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

	connect(showUnjoinableGamesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showUnjoinableGamesChanged(int)));
	connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
	connect(spectateButton, SIGNAL(clicked()), this, SLOT(actJoin()));
}

void GameSelector::showUnjoinableGamesChanged(int state)
{
	gameListProxyModel->setUnjoinableGamesVisible(state);
}

void GameSelector::actCreate()
{
	DlgCreateGame dlg(client, room->getRoomId(), room->getGameTypes(), this);
	dlg.exec();
}

void GameSelector::checkResponse(ResponseCode response)
{
	if (createButton)
		createButton->setEnabled(true);
	joinButton->setEnabled(true);
	spectateButton->setEnabled(true);

	switch (response) {
		case RespNotInRoom: QMessageBox::critical(this, tr("Error"), tr("Please join the appropriate room first.")); break;
		case RespWrongPassword: QMessageBox::critical(this, tr("Error"), tr("Wrong password.")); break;
		case RespSpectatorsNotAllowed: QMessageBox::critical(this, tr("Error"), tr("Spectators are not allowed in this game.")); break;
		case RespGameFull: QMessageBox::critical(this, tr("Error"), tr("The game is already full.")); break;
		case RespNameNotFound: QMessageBox::critical(this, tr("Error"), tr("The game does not exist any more.")); break;
		case RespUserLevelTooLow: QMessageBox::critical(this, tr("Error"), tr("This game is only open to registered users.")); break;
		case RespOnlyBuddies: QMessageBox::critical(this, tr("Error"), tr("This game is only open to its creator's buddies.")); break;
		case RespInIgnoreList: QMessageBox::critical(this, tr("Error"), tr("You are being ignored by the creator of this game.")); break;
		default: ;
	}
}

void GameSelector::actJoin()
{
	bool spectator = sender() == spectateButton;
	
	QModelIndex ind = gameListView->currentIndex();
	if (!ind.isValid())
		return;
	ServerInfo_Game *game = gameListModel->getGame(ind.data(Qt::UserRole).toInt());
	bool overrideRestrictions = !tabSupervisor->getAdminLocked();
	QString password;
	if (game->getHasPassword() && !(spectator && !game->getSpectatorsNeedPassword()) && !overrideRestrictions) {
		bool ok;
		password = QInputDialog::getText(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
		if (!ok)
			return;
	}

	Command_JoinGame *commandJoinGame = new Command_JoinGame(game->getRoomId(), game->getGameId(), password, spectator, overrideRestrictions);
	connect(commandJoinGame, SIGNAL(finished(ResponseCode)), this, SLOT(checkResponse(ResponseCode)));
	client->sendCommand(commandJoinGame);

	if (createButton)
		createButton->setEnabled(false);
	joinButton->setEnabled(false);
	spectateButton->setEnabled(false);
}

void GameSelector::retranslateUi()
{
	setTitle(tr("Games"));
	showUnjoinableGamesCheckBox->setText(tr("Show u&njoinable games"));
	if (createButton)
		createButton->setText(tr("C&reate"));
	joinButton->setText(tr("&Join"));
	spectateButton->setText(tr("J&oin as spectator"));
}

void GameSelector::processGameInfo(ServerInfo_Game *info)
{
	gameListModel->updateGameList(info);
}
 
