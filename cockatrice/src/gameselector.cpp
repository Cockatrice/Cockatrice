#include <QtGui>
#include "gameselector.h"
#include "dlg_creategame.h"

GameSelector::GameSelector(Client *_client, QWidget *parent)
	: QWidget(parent), client(_client)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(this);
	gameListView->setModel(gameListModel);

	createButton = new QPushButton(tr("C&reate"));
	joinButton = new QPushButton(tr("&Join"));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(createButton);
	buttonLayout->addWidget(joinButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gameListView);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setMinimumWidth(gameListView->columnWidth(0) * gameListModel->columnCount());
	setMinimumHeight(400);

	connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));

	connect(client, SIGNAL(gameListEvent(ServerGame *)), gameListModel, SLOT(updateGameList(ServerGame *)));
	client->listGames();
}

void GameSelector::actCreate()
{
	DlgCreateGame dlg(client, this);
	if (dlg.exec())
		deleteLater();
}

void GameSelector::actRefresh()
{
	client->listGames();
}

void GameSelector::checkResponse(ServerResponse response)
{
	createButton->setEnabled(true);
	joinButton->setEnabled(true);

	if (response == RespOk)
		deleteLater();
	else {
		QMessageBox::critical(this, tr("Error"), tr("XXX"));
		return;
	}
}

void GameSelector::actJoin()
{
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

	PendingCommand *joinCommand = client->joinGame(game->getGameId(), password);
	connect(joinCommand, SIGNAL(finished(ServerResponse)), this, SLOT(checkResponse(ServerResponse)));
	createButton->setEnabled(false);
	joinButton->setEnabled(false);
}
