#include <QtGui>
#include "dlg_creategame.h"

DlgCreateGame::DlgCreateGame(Client *_client, QWidget *parent)
	: QDialog(parent), client(_client)
{
	descriptionLabel = new QLabel(tr("&Description:"));
	descriptionEdit = new QLineEdit;
	descriptionLabel->setBuddy(descriptionEdit);

	passwordLabel = new QLabel(tr("&Password:"));
	passwordEdit = new QLineEdit;
	passwordLabel->setBuddy(passwordEdit);

	maxPlayersLabel = new QLabel(tr("P&layers:"));
	maxPlayersEdit = new QLineEdit("2");
	maxPlayersLabel->setBuddy(maxPlayersEdit);
	
	spectatorsAllowedCheckBox = new QCheckBox(tr("&Spectators allowed"));
	spectatorsAllowedCheckBox->setChecked(true);

	QGridLayout *grid = new QGridLayout;
	grid->addWidget(descriptionLabel, 0, 0);
	grid->addWidget(descriptionEdit, 0, 1);
	grid->addWidget(passwordLabel, 1, 0);
	grid->addWidget(passwordEdit, 1, 1);
	grid->addWidget(maxPlayersLabel, 2, 0);
	grid->addWidget(maxPlayersEdit, 2, 1);
	grid->addWidget(spectatorsAllowedCheckBox, 3, 0, 1, 2);

	okButton = new QPushButton(tr("&OK"));
	okButton->setDefault(true);
	cancelButton = new QPushButton(tr("&Cancel"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(grid);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);

	setWindowTitle(tr("Create game"));
	setFixedHeight(sizeHint().height());

	connect(okButton, SIGNAL(clicked()), this, SLOT(actOK()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void DlgCreateGame::actOK()
{
	bool ok;
	int maxPlayers = maxPlayersEdit->text().toInt(&ok);
	if (!ok) {
		QMessageBox::critical(this, tr("Error"), tr("Invalid number of players."));
		return;
	}
	PendingCommand *createCommand = client->createGame(descriptionEdit->text(), passwordEdit->text(), maxPlayers, spectatorsAllowedCheckBox->isChecked());
	connect(createCommand, SIGNAL(finished(ServerResponse)), this, SLOT(checkResponse(ServerResponse)));
	okButton->setEnabled(false);
	cancelButton->setEnabled(false);
}

void DlgCreateGame::checkResponse(ServerResponse response)
{
	okButton->setEnabled(true);
	cancelButton->setEnabled(true);

	if (response == RespOk)
		accept();
	else {
		QMessageBox::critical(this, tr("Error"), tr("Server error."));
		return;
	}
}
