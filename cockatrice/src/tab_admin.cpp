#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include "tab_admin.h"
#include "abstractclient.h"
#include "protocol_items.h"

ShutdownDialog::ShutdownDialog(QWidget *parent)
	: QDialog(parent)
{
	QLabel *reasonLabel = new QLabel(tr("&Reason for shutdown:"));
	reasonEdit = new QLineEdit;
	reasonLabel->setBuddy(reasonEdit);
	QLabel *minutesLabel = new QLabel(tr("&Time until shutdown (minutes):"));
	minutesEdit = new QSpinBox;
	minutesLabel->setBuddy(minutesEdit);
	minutesEdit->setMinimum(0);
	minutesEdit->setValue(5);
	
	QPushButton *okButton = new QPushButton(tr("&OK"));
	okButton->setAutoDefault(true);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);
	
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(reasonLabel, 0, 0);
	mainLayout->addWidget(reasonEdit, 0, 1);
	mainLayout->addWidget(minutesLabel, 1, 0);
	mainLayout->addWidget(minutesEdit, 1, 1);
	mainLayout->addLayout(buttonLayout, 2, 0, 1, 2);
	
	setLayout(mainLayout);
	setWindowTitle(tr("Shut down server"));
}

QString ShutdownDialog::getReason() const
{
	return reasonEdit->text();
}

int ShutdownDialog::getMinutes() const
{
	return minutesEdit->value();
}

TabAdmin::TabAdmin(TabSupervisor *_tabSupervisor, AbstractClient *_client, bool _fullAdmin, QWidget *parent)
	: Tab(_tabSupervisor, parent), locked(true), client(_client), fullAdmin(_fullAdmin)
{
	updateServerMessageButton = new QPushButton;
	connect(updateServerMessageButton, SIGNAL(clicked()), this, SLOT(actUpdateServerMessage()));
	shutdownServerButton = new QPushButton;
	connect(shutdownServerButton, SIGNAL(clicked()), this, SLOT(actShutdownServer()));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(updateServerMessageButton);
	vbox->addWidget(shutdownServerButton);
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
	shutdownServerButton->setText(tr("&Shut down server"));
	adminGroupBox->setTitle(tr("Server administration functions"));
	
	unlockButton->setText(tr("&Unlock functions"));
	lockButton->setText(tr("&Lock functions"));
}

void TabAdmin::actUpdateServerMessage()
{
	client->sendCommand(new Command_UpdateServerMessage());
}

void TabAdmin::actShutdownServer()
{
	ShutdownDialog dlg;
	if (dlg.exec())
		client->sendCommand(new Command_ShutdownServer(dlg.getReason(), dlg.getMinutes()));
}

void TabAdmin::actUnlock()
{
	if (QMessageBox::question(this, tr("Unlock administration functions"), tr("Do you really want to unlock the administration functions?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
		if (fullAdmin)
			adminGroupBox->setEnabled(true);
		lockButton->setEnabled(true);
		unlockButton->setEnabled(false);
		locked = false;
	}
}

void TabAdmin::actLock()
{
	if (fullAdmin)
		adminGroupBox->setEnabled(false);
	lockButton->setEnabled(false);
	unlockButton->setEnabled(true);
	locked = true;
}