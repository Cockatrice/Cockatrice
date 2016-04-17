#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_register.h"
#include "settingscache.h"
#include "pb/serverinfo_user.pb.h"

DlgRegister::DlgRegister(QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("&Host:"));
    QStringList hostList = settingsCache->servers().getPreviousHostList();
    hostEdit = new QComboBox();
    hostEdit->addItems(hostList);
    hostEdit->setCurrentIndex(settingsCache->servers().getPrevioushostindex());
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));    
    portEdit = new QLineEdit(settingsCache->servers().getPort("4747"));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));    
    playernameEdit = new QLineEdit(settingsCache->servers().getPlayerName("Player"));
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));    
    passwordEdit = new QLineEdit(settingsCache->servers().getPassword());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);

    passwordConfirmationLabel = new QLabel(tr("Password (again):"));
    passwordConfirmationEdit = new QLineEdit();
    passwordConfirmationLabel->setBuddy(passwordConfirmationEdit);
    passwordConfirmationEdit->setEchoMode(QLineEdit::Password);

    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailLabel->setBuddy(emailEdit);

    emailConfirmationLabel = new QLabel(tr("Email (again):"));
    emailConfirmationEdit = new QLineEdit();
    emailConfirmationLabel->setBuddy(emailConfirmationEdit);

    genderLabel = new QLabel(tr("Pronouns:"));
    genderEdit = new QComboBox();
    genderLabel->setBuddy(genderEdit);
    genderEdit->insertItem(0, QPixmap("theme:genders/unknown"), tr("Neutral"));
    genderEdit->insertItem(1, QPixmap("theme:genders/male"), tr("Masculine"));
    genderEdit->insertItem(2, QPixmap("theme:genders/female"), tr("Feminine"));
    genderEdit->setCurrentIndex(0);

    countryLabel = new QLabel(tr("Country:"));
    countryEdit = new QComboBox();
    countryLabel->setBuddy(countryEdit);
    countryEdit->insertItem(0, tr("Undefined"));
    countryEdit->setCurrentIndex(0);
    QStringList countries = settingsCache->getCountries();
    foreach(QString c, countries)
        countryEdit->addItem(QPixmap("theme:countries/" + c.toLower()), c);

    realnameLabel = new QLabel(tr("Real name:"));
    realnameEdit = new QLineEdit();
    realnameLabel->setBuddy(realnameEdit);
    
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
    grid->addWidget(emailConfirmationLabel, 6, 0);
    grid->addWidget(emailConfirmationEdit, 6, 1);
    grid->addWidget(genderLabel, 7, 0);
    grid->addWidget(genderEdit, 7, 1);
    grid->addWidget(countryLabel, 8, 0);
    grid->addWidget(countryEdit, 8, 1);
    grid->addWidget(realnameLabel, 9, 0);
    grid->addWidget(realnameEdit, 9, 1);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Register to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgRegister::actOk()
{
    if (passwordEdit->text() != passwordConfirmationEdit->text())
    {
         QMessageBox::critical(this, tr("Registration Warning"), tr("Your passwords do not match, please try again."));
         return;
    }
    else if (emailConfirmationEdit->text() != emailEdit->text())
    {
        QMessageBox::critical(this, tr("Registration Warning"), tr("Your email addresses do not match, please try again."));
        return;
    }
    if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Registration Warning"), tr("The player name can't be empty."));
        return;
    }

    settingsCache->servers().setHostName(hostEdit->currentText());
    settingsCache->servers().setPort(portEdit->text());
    settingsCache->servers().setPlayerName(playernameEdit->text());
    // always save the password so it will be picked up by the connect dialog
    settingsCache->servers().setPassword(passwordEdit->text());
  
    accept();
}

void DlgRegister::actCancel()
{
    reject();
}
