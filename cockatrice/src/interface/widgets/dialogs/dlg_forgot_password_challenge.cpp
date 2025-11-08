#include "dlg_forgot_password_challenge.h"

#include "../../../client/settings/cache_settings.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <libcockatrice/utility/trice_limits.h>

DlgForgotPasswordChallenge::DlgForgotPasswordChallenge(QWidget *parent) : QDialog(parent)
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

    if (servers.getFPHostname().isEmpty() && servers.getFPPort().isEmpty() && servers.getFPPlayerName().isEmpty()) {
        QMessageBox::warning(this, tr("Reset Password Challenge Warning"),
                             tr("A problem has occurred. Please try to request a new password again."));
        reject();
    }

    infoLabel =
        new QLabel(tr("Enter the information of the server and the account you'd like to request a new password for."));
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

    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailEdit->setMaxLength(MAX_NAME_LENGTH);
    emailLabel->setBuddy(emailLabel);

    if (!servers.getFPHostname().isEmpty() && !servers.getFPPort().isEmpty() && !servers.getFPPlayerName().isEmpty()) {
        hostLabel->hide();
        hostEdit->hide();
        portLabel->hide();
        portEdit->hide();
        playernameLabel->hide();
        playernameEdit->hide();
    }

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(infoLabel, 0, 0, 1, 2);
    grid->addWidget(hostLabel, 1, 0);
    grid->addWidget(hostEdit, 1, 1);
    grid->addWidget(portLabel, 2, 0);
    grid->addWidget(portEdit, 2, 1);
    grid->addWidget(playernameLabel, 3, 0);
    grid->addWidget(playernameEdit, 3, 1);
    grid->addWidget(emailLabel, 4, 0);
    grid->addWidget(emailEdit, 4, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgForgotPasswordChallenge::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgForgotPasswordChallenge::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Reset Password Challenge"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordChallenge::actOk()
{
    if (emailEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Reset Password Challenge Error"), tr("The email address can't be empty."));
        return;
    }

    ServersSettings &servers = SettingsCache::instance().servers();
    servers.setFPHostName(hostEdit->text());
    servers.setFPPort(portEdit->text());
    servers.setFPPlayerName(playernameEdit->text());

    accept();
}
