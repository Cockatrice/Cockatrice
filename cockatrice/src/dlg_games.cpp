#include <QtGui>
#include "dlg_games.h"
#include "dlg_creategame.h"

DlgGames::DlgGames(Client *_client, QWidget *parent)
	: QDialog(parent), client(_client), msgid(0)
{
	gameListView = new QTreeView;
	gameListModel = new GamesModel(this);
	gameListView->setModel(gameListModel);
	
	createButton = new QPushButton(tr("&Create"));
	joinButton = new QPushButton(tr("&Join"));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(createButton);
	buttonLayout->addWidget(joinButton);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gameListView);
	mainLayout->addLayout(buttonLayout);
	
	setLayout(mainLayout);
	setWindowTitle(tr("Games"));

	setMinimumWidth(gameListView->columnWidth(0) * gameListModel->columnCount());
	connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
	
	connect(client, SIGNAL(gameListEvent(ServerGame *)), gameListModel, SLOT(updateGameList(ServerGame *)));
	client->listGames();
}

void DlgGames::actCreate()
{
	DlgCreateGame dlg(client, this);
	if (dlg.exec())
		accept();
}

void DlgGames::actRefresh()
{
	client->listGames();
}

void DlgGames::checkResponse(ServerResponse *response)
{
	if (response->getMsgId() != msgid)
		return;
	if (response->getOk())
		accept();
	else {
		QMessageBox::critical(this, tr("Error"), tr("XXX"));
		msgid = 0;
		return;
	}
}

void DlgGames::actJoin()
{
	if (msgid)
		return;
		
	ServerGame *game = gameListModel->getGame(gameListView->currentIndex().row());
	QString password;
	if (game->getHasPassword()) {
		bool ok;
		password = QInputDialog::getText(this, tr("Join game"), tr("Password:"), QLineEdit::Password, QString(), &ok);
		if (!ok)
			return;
	}
	
	connect(client, SIGNAL(responseReceived(ServerResponse *)), this, SLOT(checkResponse(ServerResponse *)));
	msgid = client->joinGame(game->getGameId(), password);
}
