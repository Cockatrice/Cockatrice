#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include "tab_admin.h"
#include "abstractclient.h"
#include "protocol_items.h"

TabAdmin::TabAdmin(AbstractClient *_client, QWidget *parent)
	: Tab(parent), client(_client)
{
	updateServerMessageButton = new QPushButton;
	connect(updateServerMessageButton, SIGNAL(clicked()), this, SLOT(actUpdateServerMessage()));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(updateServerMessageButton);
	vbox->addStretch();
	
	adminGroupBox = new QGroupBox;
	adminGroupBox->setLayout(vbox);
	adminGroupBox->setEnabled(false);
	
	unlockButton = new QPushButton;
	connect(unlockButton, SIGNAL(clicked()), this, SLOT(actUnlock()));
	lockButton = new QPushButton;
	lockButton->setEnabled(false);
	connect(lockButton, SIGNAL(clicked()), this, SLOT(actLock()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(adminGroupBox);
	mainLayout->addWidget(unlockButton);
	mainLayout->addWidget(lockButton);
	
	retranslateUi();
	setLayout(mainLayout);
}

void TabAdmin::retranslateUi()
{
	updateServerMessageButton->setText(tr("Update server &message"));
	adminGroupBox->setTitle(tr("Server administration functions"));
	
	unlockButton->setText(tr("&Unlock functions"));
	lockButton->setText(tr("&Lock functions"));
}

void TabAdmin::actUpdateServerMessage()
{
	client->sendCommand(new Command_UpdateServerMessage());
}

void TabAdmin::actUnlock()
{
	if (QMessageBox::question(this, tr("Unlock administration functions"), tr("Do you really want to unlock the administration functions?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
		adminGroupBox->setEnabled(true);
		lockButton->setEnabled(true);
		unlockButton->setEnabled(false);
	}
}

void TabAdmin::actLock()
{
	adminGroupBox->setEnabled(false);
	lockButton->setEnabled(false);
	unlockButton->setEnabled(true);
}