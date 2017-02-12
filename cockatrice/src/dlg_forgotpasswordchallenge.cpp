#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_forgotpasswordchallenge.h"
#include "settingscache.h"

DlgForgotPasswordChallenge::DlgForgotPasswordChallenge(QWidget *parent)
    : QDialog(parent)
{

    QString lastfphost; QString lastfpport; QString lastfpplayername;
    lastfphost = settingsCache->servers().getHostname("cockatrice.woogerworks.com");
    lastfpport = settingsCache->servers().getPort("4747");
    lastfpplayername = settingsCache->servers().getPlayerName("Player");

    if (!settingsCache->servers().getFPHostname().isEmpty() && !settingsCache->servers().getFPPort().isEmpty() && !settingsCache->servers().getFPPlayerName().isEmpty()) {
        lastfphost = settingsCache->servers().getFPHostname();
        lastfpport = settingsCache->servers().getFPPort();
        lastfpplayername = settingsCache->servers().getFPPlayerName();
    }

    if (settingsCache->servers().getFPHostname().isEmpty() && settingsCache->servers().getFPPort().isEmpty() && settingsCache->servers().getFPPlayerName().isEmpty())
    {
        QMessageBox::warning(this, tr("Forgot Password Challenge Warning"), tr("Oops, looks like something has gone wrong.  Please restart the forgot password process by using the forgot password button on the connection screen."));
        actCancel();
    }

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(lastfphost);
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(lastfpport);
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(lastfpplayername);
    playernameLabel->setBuddy(playernameEdit);

    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailLabel->setBuddy(emailLabel);

    if (!settingsCache->servers().getFPHostname().isEmpty() && !settingsCache->servers().getFPPort().isEmpty() && !settingsCache->servers().getFPPlayerName().isEmpty()) {
        hostLabel->hide();
        hostEdit->hide();
        portLabel->hide();
        portEdit->hide();
        playernameLabel->hide();
        playernameEdit->hide();
    }

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(hostLabel, 0, 0);
    grid->addWidget(hostEdit, 0, 1);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(portEdit, 1, 1);
    grid->addWidget(playernameLabel, 2, 0);
    grid->addWidget(playernameEdit, 2, 1);
    grid->addWidget(emailLabel, 3, 0);
    grid->addWidget(emailEdit, 3, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Forgot Password Challenge"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordChallenge::actOk()
{
    if (emailEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Forgot Password Challenge Warning"), tr("The email address can't be empty."));
        return;
    }

    settingsCache->servers().setFPHostName(hostEdit->text());
    settingsCache->servers().setFPPort(portEdit->text());
    settingsCache->servers().setFPPlayerName(playernameEdit->text());

    accept();
}

void DlgForgotPasswordChallenge::actCancel()
{
    reject();
}
