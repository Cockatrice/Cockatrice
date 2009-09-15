#include <QtGui>
#include "gameselector.h"
#include "dlg_creategame.h"

GameSelector::GameSelector(Client *_client, QWidget *parent)
	: QWidget(parent), client(_client)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(this);
	gameListView->setModel(gameListModel);

	createButton = new QPushButton;
	joinButton = new QPushButton;
	spectateButton = new QPushButton;
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(createButton);
	buttonLayout->addWidget(joinButton);
	buttonLayout->addWidget(spectateButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gameListView);
	mainLayout->addLayout(buttonLayout);

	retranslateUi();
	setLayout(mainLayout);

	setMinimumWidth(gameListView->columnWidth(0) * gameListModel->columnCount());
	setMinimumHeight(400);

	connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
	connect(spectateButton, SIGNAL(clicked()), this, SLOT(actJoin()));
}

void GameSelector::actCreate()
{
	DlgCreateGame dlg(client, this);
	if (dlg.exec())
		disableGameList();
}

void GameSelector::actRefresh()
{
	client->listGames();
}

void GameSelector::checkResponse(ServerResponse response)
{
	createButton->setEnabled(true);
	joinButton->setEnabled(true);
	spectateButton->setEnabled(true);

	if (response == RespOk)
		disableGameList();
	else {
		QMessageBox::critical(this, tr("Error"), tr("XXX"));
		return;
	}
}

void GameSelector::actJoin()
{
	bool spectator = sender() == spectateButton;
	
	QModelIndex ind = gameListView->currentIndex();
	if (!ind.isValid())
		return;
	ServerGame *game = gameListModel->getGame(ind.row());
	QString password;
	if (game->getHasPassword()) {
		bool ok;
		password = QInputDialog::getText(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
		if (!ok)
			return;
	}

	PendingCommand *joinCommand = client->joinGame(game->getGameId(), password, spectator);
	connect(joinCommand, SIGNAL(finished(ServerResponse)), this, SLOT(checkResponse(ServerResponse)));
	createButton->setEnabled(false);
	joinButton->setEnabled(false);
	spectateButton->setEnabled(false);
}

void GameSelector::enableGameList()
{
	if (isVisible())
		return;
	
	connect(client, SIGNAL(gameListEvent(ServerGame *)), gameListModel, SLOT(updateGameList(ServerGame *)));
	client->listGames();
	show();
}

void GameSelector::disableGameList()
{
	if (!isVisible())
		return;
	
	disconnect(client, 0, gameListModel, 0);
	hide();
	gameListModel->cleanList();
}

void GameSelector::retranslateUi()
{
	createButton->setText(tr("C&reate"));
	joinButton->setText(tr("&Join"));
	spectateButton->setText(tr("J&oin as spectator"));
}