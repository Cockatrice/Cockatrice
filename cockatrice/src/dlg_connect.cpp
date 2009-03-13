#include <QtGui>
#include "dlg_connect.h"

DlgConnect::DlgConnect(QWidget *parent)
	: QDialog(parent)
{
	hostLabel = new QLabel(tr("&Host:"));
	hostEdit = new QLineEdit("localhost");
	hostLabel->setBuddy(hostEdit);
	
	portLabel = new QLabel(tr("&Port:"));
	portEdit = new QLineEdit("4747");
	portLabel->setBuddy(portEdit);

	playernameLabel = new QLabel(tr("Player &name:"));
	playernameEdit = new QLineEdit("Player");
	playernameLabel->setBuddy(playernameEdit);
	
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
	
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QString DlgConnect::getHost()
{
	return hostEdit->text();
}

int DlgConnect::getPort()
{
	return portEdit->text().toInt();
}

QString DlgConnect::getPlayerName()
{
	return playernameEdit->text();
}
