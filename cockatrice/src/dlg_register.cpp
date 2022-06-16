#include "dlg_register.h"

#include "pb/serverinfo_user.pb.h"
#include "settingscache.h"
#include "stringsizes.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

DlgRegister::DlgRegister(QWidget *parent) : QDialog(parent)
{
    ServersSettings &servers = SettingsCache::instance().servers();
    infoLabel = new QLabel(tr("Enter your information and the information of the server you'd like to register to.\n"
                              "Your email will be used to verify your account."));
    infoLabel->setWordWrap(true);

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(servers.getHostname());
    hostEdit->setMaxLength(MAX_NAME_LENGTH);
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(servers.getPort());
    portEdit->setValidator(new QIntValidator(0, 0xffff, portEdit));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(servers.getPlayerName());
    playernameEdit->setMaxLength(MAX_NAME_LENGTH);
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit();
    passwordEdit->setMaxLength(MAX_NAME_LENGTH);
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);

    emailConfirmationLabel = new QLabel(tr("Email (again):"));
    emailConfirmationEdit = new QLineEdit();
    emailConfirmationEdit->setMaxLength(MAX_NAME_LENGTH);
    emailConfirmationLabel->setBuddy(emailConfirmationEdit);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(infoLabel, 0, 0, 1, 2);
    grid->addWidget(hostLabel, 1, 0);
    grid->addWidget(hostEdit, 1, 1);
    grid->addWidget(portLabel, 2, 0);
    grid->addWidget(portEdit, 2, 1);
    grid->addWidget(playernameLabel, 3, 0);
    grid->addWidget(playernameEdit, 3, 1);
    grid->addWidget(passwordLabel, 4, 0);
    grid->addWidget(passwordEdit, 4, 1);
    grid->addWidget(passwordConfirmationLabel, 5, 0);
    grid->addWidget(passwordConfirmationEdit, 5, 1);
    grid->addWidget(emailLabel, 6, 0);
    grid->addWidget(emailEdit, 6, 1);
    grid->addWidget(emailConfirmationLabel, 7, 0);
    grid->addWidget(emailConfirmationEdit, 7, 1);
    grid->addWidget(countryLabel, 9, 0);
    grid->addWidget(countryEdit, 9, 1);
    grid->addWidget(realnameLabel, 10, 0);
    grid->addWidget(realnameEdit, 10, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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
    // TODO this stuff should be using qvalidators
    if (passwordEdit->text().length() < 8) {
        QMessageBox::critical(this, tr("Registration Warning"), tr("Your password is too short."));
        return;
    } else if (passwordEdit->text() != passwordConfirmationEdit->text()) {
        QMessageBox::critical(this, tr("Registration Warning"), tr("Your passwords do not match, please try again."));
        return;
    } else if (emailConfirmationEdit->text() != emailEdit->text()) {
        QMessageBox::critical(this, tr("Registration Warning"),
                              tr("Your email addresses do not match, please try again."));
        return;
    }
    if (playernameEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Registration Warning"), tr("The player name can't be empty."));
        return;
    }

    accept();
}
