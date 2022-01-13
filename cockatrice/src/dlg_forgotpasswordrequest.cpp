#include "dlg_forgotpasswordrequest.h"

#include "settingscache.h"
#include "stringsizes.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

DlgForgotPasswordRequest::DlgForgotPasswordRequest(QWidget *parent) : QDialog(parent)
{
    QString lastfphost;
    QString lastfpport;
    QString lastfpplayername;
    ServersSettings &servers = SettingsCache::instance().servers();
    lastfphost = servers.getHostname();
    lastfpport = servers.getPort();
    lastfpplayername = servers.getPlayerName();

    if (!servers.getFPHostname().isEmpty() && !servers.getFPPort().isEmpty() && !servers.getFPPlayerName().isEmpty()) {
        lastfphost = servers.getFPHostname();
        lastfpport = servers.getFPPort();
        lastfpplayername = servers.getFPPlayerName();
    }

    infoLabel = new QLabel(tr("Enter the information of the server you'd like to request a new password for."));
    infoLabel->setWordWrap(true);

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(lastfphost);
    hostEdit->setMaxLength(MAX_NAME_LENGTH);
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(lastfpport);
    portEdit->setValidator(new QIntValidator(0, 0xffff, portEdit));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(lastfpplayername);
    playernameEdit->setMaxLength(MAX_NAME_LENGTH);
    playernameLabel->setBuddy(playernameEdit);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(infoLabel, 0, 0, 1, 2);
    grid->addWidget(hostLabel, 1, 0);
    grid->addWidget(hostEdit, 1, 1);
    grid->addWidget(portLabel, 2, 0);
    grid->addWidget(portEdit, 2, 1);
    grid->addWidget(playernameLabel, 3, 0);
    grid->addWidget(playernameEdit, 3, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Reset Password Request"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordRequest::actOk()
{
    if (playernameEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Reset Password Error"), tr("The player name can't be empty."));
        return;
    }

    ServersSettings &servers = SettingsCache::instance().servers();
    servers.setFPHostName(hostEdit->text());
    servers.setFPPort(portEdit->text());
    servers.setFPPlayerName(playernameEdit->text());

    accept();
}
