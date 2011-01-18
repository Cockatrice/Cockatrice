#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QMessageBox>
#include "dlg_creategame.h"
#include "protocol_items.h"

DlgCreateGame::DlgCreateGame(AbstractClient *_client, int _roomId, QWidget *parent)
	: QDialog(parent), client(_client), roomId(_roomId)
{
	descriptionLabel = new QLabel(tr("&Description:"));
	descriptionEdit = new QLineEdit;
	descriptionLabel->setBuddy(descriptionEdit);

	passwordLabel = new QLabel(tr("&Password:"));
	passwordEdit = new QLineEdit;
	passwordLabel->setBuddy(passwordEdit);

	maxPlayersLabel = new QLabel(tr("P&layers:"));
	maxPlayersEdit = new QSpinBox();
	maxPlayersEdit->setMinimum(1);
	maxPlayersEdit->setMaximum(100);
	maxPlayersEdit->setValue(2);
	maxPlayersLabel->setBuddy(maxPlayersEdit);
	
	spectatorsAllowedCheckBox = new QCheckBox(tr("&Spectators allowed"));
	spectatorsAllowedCheckBox->setChecked(true);
	connect(spectatorsAllowedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(spectatorsAllowedChanged(int)));
	spectatorsNeedPasswordCheckBox = new QCheckBox(tr("Spectators &need a password to join"));
	spectatorsCanTalkCheckBox = new QCheckBox(tr("Spectators can &chat"));
	spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Spectators see &everything"));
	QVBoxLayout *spectatorsLayout = new QVBoxLayout;
	spectatorsLayout->addWidget(spectatorsAllowedCheckBox);
	spectatorsLayout->addWidget(spectatorsNeedPasswordCheckBox);
	spectatorsLayout->addWidget(spectatorsCanTalkCheckBox);
	spectatorsLayout->addWidget(spectatorsSeeEverythingCheckBox);
	spectatorsGroupBox = new QGroupBox(tr("Spectators"));
	spectatorsGroupBox->setLayout(spectatorsLayout);

	QGridLayout *grid = new QGridLayout;
	grid->addWidget(descriptionLabel, 0, 0);
	grid->addWidget(descriptionEdit, 0, 1);
	grid->addWidget(passwordLabel, 1, 0);
	grid->addWidget(passwordEdit, 1, 1);
	grid->addWidget(maxPlayersLabel, 2, 0);
	grid->addWidget(maxPlayersEdit, 2, 1);
	grid->addWidget(spectatorsGroupBox, 3, 0, 1, 2);

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
	Command_CreateGame *createCommand = new Command_CreateGame(
		roomId,
		descriptionEdit->text(),
		passwordEdit->text(),
		maxPlayersEdit->value(),
		spectatorsAllowedCheckBox->isChecked(),
		spectatorsNeedPasswordCheckBox->isChecked(),
		spectatorsCanTalkCheckBox->isChecked(),
		spectatorsSeeEverythingCheckBox->isChecked()
	);
	connect(createCommand, SIGNAL(finished(ResponseCode)), this, SLOT(checkResponse(ResponseCode)));
	client->sendCommand(createCommand);
	
	okButton->setEnabled(false);
	cancelButton->setEnabled(false);
}

void DlgCreateGame::checkResponse(ResponseCode response)
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

void DlgCreateGame::spectatorsAllowedChanged(int state)
{
	spectatorsNeedPasswordCheckBox->setEnabled(state);
	spectatorsCanTalkCheckBox->setEnabled(state);
	spectatorsSeeEverythingCheckBox->setEnabled(state);
}
