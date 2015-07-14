#include <QSettings>
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
    QSettings settings;
    settings.beginGroup("server");

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settings.value("hostname", "cockatrice.woogerworks.com").toString());
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
    genderEdit->insertItem(0, QIcon(":/resources/genders/unknown.svg"), tr("Neutral"));
    genderEdit->insertItem(1, QIcon(":/resources/genders/male.svg"), tr("Masculine"));
    genderEdit->insertItem(2, QIcon(":/resources/genders/female.svg"), tr("Feminine"));
    genderEdit->setCurrentIndex(0);

    countryLabel = new QLabel(tr("Country:"));
    countryEdit = new QComboBox();
    countryLabel->setBuddy(countryEdit);
    countryEdit->insertItem(0, tr("Undefined"));
    countryEdit->setCurrentIndex(0);
    QStringList countries = settingsCache->getCountries();
    foreach(QString c, countries)
    {
        countryEdit->addItem(QPixmap(":/resources/countries/" + c + ".svg"), c);
    }

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

    QSettings settings;
    settings.beginGroup("server");
    settings.setValue("hostname", hostEdit->text());
    settings.setValue("port", portEdit->text());
    settings.setValue("playername", playernameEdit->text());
    // always save the password so it will be picked up by the connect dialog
    settings.setValue("password", passwordEdit->text());
    settings.endGroup();
  
    accept();
}

void DlgRegister::actCancel()
{
    reject();
}
