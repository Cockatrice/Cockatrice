#include "dlg_forgotpasswordreset.h"

#include "settingscache.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

DlgForgotPasswordReset::DlgForgotPasswordReset(QWidget *parent) : QDialog(parent)
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

    if (servers.getFPHostname().isEmpty() && servers.getFPPort().isEmpty() && servers.getFPPlayerName().isEmpty()) {
        QMessageBox::warning(this, tr("Forgot Password Reset Warning"),
                             tr("Oops, looks like something has gone wrong. Please re-start the forgot password "
                                "process by using the forgot password button on the connection screen."));
        reject();
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

    tokenLabel = new QLabel(tr("Token:"));
    tokenEdit = new QLineEdit();
    tokenLabel->setBuddy(tokenLabel);

    newpasswordLabel = new QLabel(tr("New Password:"));
    newpasswordEdit = new QLineEdit();
    newpasswordLabel->setBuddy(newpasswordEdit);
    newpasswordEdit->setEchoMode(QLineEdit::Password);

    newpasswordverifyLabel = new QLabel(tr("New Password:"));
    newpasswordverifyEdit = new QLineEdit();
    newpasswordverifyLabel->setBuddy(newpasswordEdit);
    newpasswordverifyEdit->setEchoMode(QLineEdit::Password);

    if (!servers.getFPHostname().isEmpty() && !servers.getFPPort().isEmpty() && !servers.getFPPlayerName().isEmpty()) {
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
    grid->addWidget(tokenLabel, 3, 0);
    grid->addWidget(tokenEdit, 3, 1);
    grid->addWidget(newpasswordLabel, 4, 0);
    grid->addWidget(newpasswordEdit, 4, 1);
    grid->addWidget(newpasswordverifyLabel, 5, 0);
    grid->addWidget(newpasswordverifyEdit, 5, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Forgot Password Reset"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordReset::actOk()
{
    if (playernameEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The player name can't be empty."));
        return;
    }

    if (tokenEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The token can't be empty."));
        return;
    }

    if (newpasswordEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The new password can't be empty."));
        return;
    }

    if (newpasswordEdit->text() != newpasswordverifyEdit->text()) {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The passwords do not match."));
        return;
    }

    ServersSettings &servers = SettingsCache::instance().servers();
    servers.setFPHostName(hostEdit->text());
    servers.setFPPort(portEdit->text());
    servers.setFPPlayerName(playernameEdit->text());

    accept();
}
