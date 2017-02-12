#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_forgotpasswordrequest.h"
#include "settingscache.h"

DlgForgotPasswordRequest::DlgForgotPasswordRequest(QWidget *parent)
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

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(lastfphost);
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(lastfpport);
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(lastfpplayername);
    playernameLabel->setBuddy(playernameEdit);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(hostLabel, 0, 0);
    grid->addWidget(hostEdit, 0, 1);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(portEdit, 1, 1);
    grid->addWidget(playernameLabel, 2, 0);
    grid->addWidget(playernameEdit, 2, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Forgot Password Request"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordRequest::actOk()
{
    if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Forgot Password Request Warning"), tr("The player name can't be empty."));
        return;
    }

    settingsCache->servers().setFPHostName(hostEdit->text());
    settingsCache->servers().setFPPort(portEdit->text());
    settingsCache->servers().setFPPlayerName(playernameEdit->text());

    accept();
}

void DlgForgotPasswordRequest::actCancel()
{
    reject();
}
