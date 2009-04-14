#include <QtGui>
#include "dlg_games.h"
#include "dlg_creategame.h"

DlgGames::DlgGames(Client *_client, QWidget *parent)
	: QDialog(parent), client(_client), msgid(0)
{
	tableView = new QTreeView;
	tableModel = new GamesModel(this);
	tableView->setModel(tableModel);
	
	createButton = new QPushButton(tr("&Create"));
	refreshButton = new QPushButton(tr("&Refresh"));
	joinButton = new QPushButton(tr("&Join"));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(createButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(refreshButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(joinButton);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tableView);
	mainLayout->addLayout(buttonLayout);
	
	setLayout(mainLayout);
	
	setWindowTitle(tr("Games"));

	setMinimumWidth(tableView->columnWidth(0) * tableModel->columnCount());
	connect(createButton, SIGNAL(clicked()), this, SLOT(actCreate()));
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(actRefresh()));
	connect(joinButton, SIGNAL(clicked()), this, SLOT(actJoin()));
	
	connect(client, SIGNAL(gameListReceived(QList<ServerGame *>)), this, SLOT(gameListReceived(QList<ServerGame *>)));
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
		
	ServerGame *game = tableModel->getGame(tableView->currentIndex().row());
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

void DlgGames::gameListReceived(QList<ServerGame *> _gameList)
{
	tableModel->setGameList(_gameList);
}
