#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_forgotpassword.h"
#include "settingscache.h"
#include "pb/serverinfo_user.pb.h"

DlgForgotPassword::DlgForgotPassword(QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settingsCache->servers().getHostname("cockatrice.woogerworks.com"));
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));    
    portEdit = new QLineEdit(settingsCache->servers().getPort("4747"));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));    
    playernameEdit = new QLineEdit(settingsCache->servers().getPlayerName("Player"));
    playernameLabel->setBuddy(playernameEdit);

	emailLabel = new QLabel(tr("Email:"));
	emailEdit = new QLineEdit();
	emailLabel->setBuddy(emailEdit);
	
    passwordLabel = new QLabel(tr("New P&assword:"));    
    passwordEdit = new QLineEdit(settingsCache->servers().getPassword());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);

    passwordConfirmationLabel = new QLabel(tr("New Password (again):"));
    passwordConfirmationEdit = new QLineEdit();
    passwordConfirmationLabel->setBuddy(passwordConfirmationEdit);
    passwordConfirmationEdit->setEchoMode(QLineEdit::Password);
	 

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(hostLabel, 0, 0);
    grid->addWidget(hostEdit, 0, 1);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(portEdit, 1, 1);
    grid->addWidget(playernameLabel, 2, 0);
    grid->addWidget(playernameEdit, 2, 1);
    grid->addWidget(passwordLabel, 3, 0);
    grid->addWidget(passwordEdit, 3, 1);
    grid->addWidget(passwordConfirmationLabel, 4, 0);
    grid->addWidget(passwordConfirmationEdit, 4, 1);
    grid->addWidget(emailLabel, 5, 0);
    grid->addWidget(emailEdit, 5, 1);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Forgot Password"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPassword::actOk()
{
    if (passwordEdit->text() != passwordConfirmationEdit->text())
    {
         QMessageBox::critical(this, tr("Forgot Password Warning"), tr("Your passwords do not match, please try again."));
         return;
    }
	else if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Forgot Password Warning"), tr("The player name can't be empty."));
        return;
    }

    settingsCache->servers().setHostName(hostEdit->text());
    settingsCache->servers().setPort(portEdit->text());
    settingsCache->servers().setPlayerName(playernameEdit->text());
    // always save the password so it will be picked up by the connect dialog
    settingsCache->servers().setPassword(passwordEdit->text());
  
    accept();
}

void DlgForgotPassword::actCancel()
{
    reject();
}
