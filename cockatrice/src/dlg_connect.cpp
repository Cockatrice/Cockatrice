#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include "dlg_connect.h"

DlgConnect::DlgConnect(QWidget *parent)
	: QDialog(parent)
{
	QSettings settings;
	settings.beginGroup("server");

	hostLabel = new QLabel(tr("&Host:"));
	hostEdit = new QLineEdit(settings.value("hostname", "play.cockatrice.de").toString());
	hostLabel->setBuddy(hostEdit);

	portLabel = new QLabel(tr("&Port:"));
	portEdit = new QLineEdit(settings.value("port", "4747").toString());
	portLabel->setBuddy(portEdit);

	playernameLabel = new QLabel(tr("Player &name:"));
	playernameEdit = new QLineEdit(settings.value("playername", "Player").toString());
	playernameLabel->setBuddy(playernameEdit);

	passwordLabel = new QLabel(tr("P&assword:"));
	passwordEdit = new QLineEdit(settings.value("password").toString());
	passwordLabel->setBuddy(passwordEdit);
	passwordEdit->setEchoMode(QLineEdit::Password);

	okButton = new QPushButton(tr("&OK"));
	okButton->setDefault(true);
	cancelButton = new QPushButton(tr("&Cancel"));

	QGridLayout *grid = new QGridLayout;
	grid->addWidget(hostLabel, 0, 0);
	grid->addWidget(hostEdit, 0, 1);
	grid->addWidget(portLabel, 1, 0);
	grid->addWidget(portEdit, 1, 1);
	grid->addWidget(playernameLabel, 2, 0);
	grid->addWidget(playernameEdit, 2, 1);
	grid->addWidget(passwordLabel, 3, 0);
	grid->addWidget(passwordEdit, 3, 1);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(grid);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	setWindowTitle(tr("Connect to server"));
	setFixedHeight(sizeHint().height());
	setMinimumWidth(300);

	connect(okButton, SIGNAL(clicked()), this, SLOT(actOk()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void DlgConnect::actOk()
{
	QSettings settings;
	settings.beginGroup("server");
	settings.setValue("hostname", hostEdit->text());
	settings.setValue("port", portEdit->text());
	settings.setValue("playername", playernameEdit->text());
	settings.setValue("password", passwordEdit->text());
	settings.endGroup();

	accept();
}
