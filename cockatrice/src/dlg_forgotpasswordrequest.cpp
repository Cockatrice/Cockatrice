#include "dlg_forgotpasswordrequest.h"

#include "settingscache.h"

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
    lastfphost = servers.getHostname("server.cockatrice.us");
    lastfpport = servers.getPort("4747");
    lastfpplayername = servers.getPlayerName("Player");

    if (!servers.getFPHostname().isEmpty() && !servers.getFPPort().isEmpty() && !servers.getFPPlayerName().isEmpty()) {
        lastfphost = servers.getFPHostname();
        lastfpport = servers.getFPPort();
        lastfpplayername = servers.getFPPlayerName();
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
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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
    if (playernameEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Forgot Password Request Warning"), tr("The player name can't be empty."));
        return;
    }

    ServersSettings &servers = SettingsCache::instance().servers();
    servers.setFPHostName(hostEdit->text());
    servers.setFPPort(portEdit->text());
    servers.setFPPlayerName(playernameEdit->text());

    accept();
}
