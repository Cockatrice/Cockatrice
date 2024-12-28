#include "tab_admin.h"

#include "../../server/pending_command.h"
#include "../game_logic/abstract_client.h"
#include "pb/admin_commands.pb.h"
#include "pb/event_replay_added.pb.h"
#include "pb/moderator_commands.pb.h"
#include "trice_limits.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

ShutdownDialog::ShutdownDialog(QWidget *parent) : QDialog(parent)
{
    QLabel *reasonLabel = new QLabel(tr("&Reason for shutdown:"));
    reasonEdit = new QLineEdit;
    reasonEdit->setMaxLength(MAX_TEXT_LENGTH);
    reasonLabel->setBuddy(reasonEdit);
    QLabel *minutesLabel = new QLabel(tr("&Time until shutdown (minutes):"));
    minutesEdit = new QSpinBox;
    minutesLabel->setBuddy(minutesEdit);
    minutesEdit->setMinimum(0);
    minutesEdit->setValue(5);
    minutesEdit->setMaximum(999);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ShutdownDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ShutdownDialog::reject);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(reasonLabel, 0, 0);
    mainLayout->addWidget(reasonEdit, 0, 1);
    mainLayout->addWidget(minutesLabel, 1, 0);
    mainLayout->addWidget(minutesEdit, 1, 1);
    mainLayout->addWidget(buttonBox, 2, 0, 1, 2);

    setLayout(mainLayout);
    setWindowTitle(tr("Shut down server"));
}

QString ShutdownDialog::getReason() const
{
    return reasonEdit->text();
}

int ShutdownDialog::getMinutes() const
{
    return minutesEdit->value();
}

TabAdmin::TabAdmin(TabSupervisor *_tabSupervisor, AbstractClient *_client, bool _fullAdmin, QWidget *parent)
    : Tab(_tabSupervisor, parent), locked(true), client(_client), fullAdmin(_fullAdmin)
{
    updateServerMessageButton = new QPushButton;
    connect(updateServerMessageButton, &QPushButton::clicked, this, &TabAdmin::actUpdateServerMessage);
    shutdownServerButton = new QPushButton;
    connect(shutdownServerButton, &QPushButton::clicked, this, &TabAdmin::actShutdownServer);
    reloadConfigButton = new QPushButton;
    connect(reloadConfigButton, &QPushButton::clicked, this, &TabAdmin::actReloadConfig);

    grantReplayAccessButton = new QPushButton;
    grantReplayAccessButton->setEnabled(false);
    connect(grantReplayAccessButton, &QPushButton::clicked, this, &TabAdmin::actGrantReplayAccess);
    replayIdToGrant = new QLineEdit;
    replayIdToGrant->setMaximumWidth(500);
    replayIdToGrant->setValidator(new QIntValidator(0, INT_MAX, this));
    connect(replayIdToGrant, &QLineEdit::textChanged, this,
            [=, this]() { grantReplayAccessButton->setEnabled(!replayIdToGrant->text().isEmpty()); });
    auto *grandReplayAccessLayout = new QGridLayout(this);
    grandReplayAccessLayout->addWidget(replayIdToGrant, 0, 0);
    grandReplayAccessLayout->addWidget(grantReplayAccessButton, 0, 1);

    activateUserButton = new QPushButton;
    activateUserButton->setEnabled(false);
    connect(activateUserButton, &QPushButton::clicked, this, &TabAdmin::actForceActivateUser);
    userToActivate = new QLineEdit;
    userToActivate->setMaximumWidth(500);
    connect(userToActivate, &QLineEdit::textChanged, this,
            [=, this]() { activateUserButton->setEnabled(!userToActivate->text().isEmpty()); });
    auto *activateUserLayout = new QGridLayout(this);
    activateUserLayout->addWidget(userToActivate, 0, 0);
    activateUserLayout->addWidget(activateUserButton, 0, 1);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(updateServerMessageButton);
    vbox->addWidget(shutdownServerButton);
    vbox->addWidget(reloadConfigButton);
    vbox->addLayout(grandReplayAccessLayout);
    vbox->addLayout(activateUserLayout);
    vbox->addStretch();

    adminGroupBox = new QGroupBox;
    adminGroupBox->setLayout(vbox);
    adminGroupBox->setEnabled(false);

    unlockButton = new QPushButton;
    connect(unlockButton, &QPushButton::clicked, this, &TabAdmin::actUnlock);
    lockButton = new QPushButton;
    lockButton->setEnabled(false);
    connect(lockButton, &QPushButton::clicked, this, &TabAdmin::actLock);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(adminGroupBox);
    mainLayout->addWidget(unlockButton);
    mainLayout->addWidget(lockButton);

    retranslateUi();

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    actUnlock();
}

void TabAdmin::retranslateUi()
{
    updateServerMessageButton->setText(tr("Update server &message"));
    shutdownServerButton->setText(tr("&Shut down server"));
    reloadConfigButton->setText(tr("&Reload configuration"));
    adminGroupBox->setTitle(tr("Server administration functions"));

    replayIdToGrant->setPlaceholderText(tr("Replay ID"));
    grantReplayAccessButton->setText(tr("Grant Replay Access"));

    userToActivate->setPlaceholderText(tr("Username to Activate"));
    activateUserButton->setText(tr("Force Activate User"));

    unlockButton->setText(tr("&Unlock functions"));
    lockButton->setText(tr("&Lock functions"));
}

void TabAdmin::actUpdateServerMessage()
{
    client->sendCommand(client->prepareAdminCommand(Command_UpdateServerMessage()));
}

void TabAdmin::actShutdownServer()
{
    ShutdownDialog dlg;
    if (dlg.exec()) {
        Command_ShutdownServer cmd;
        cmd.set_reason(dlg.getReason().toStdString());
        cmd.set_minutes(dlg.getMinutes());

        client->sendCommand(AbstractClient::prepareAdminCommand(cmd));
    }
}

void TabAdmin::actReloadConfig()
{
    Command_ReloadConfig cmd;
    client->sendCommand(client->prepareAdminCommand(cmd));
}

void TabAdmin::actGrantReplayAccess()
{
    if (!replayIdToGrant) {
        return;
    }

    Command_GrantReplayAccess cmd;
    cmd.set_replay_id(replayIdToGrant->text().toUInt());
    cmd.set_moderator_name(client->getUserName().toStdString());

    auto *pend = client->prepareModeratorCommand(cmd);
    connect(pend,
            QOverload<const Response &, const CommandContainer &, const QVariant &>::of(&PendingCommand::finished),
            this, &TabAdmin::grantReplayAccessProcessResponse);
    client->sendCommand(pend);
}

void TabAdmin::actForceActivateUser()
{
    if (!userToActivate) {
        return;
    }

    Command_ForceActivateUser cmd;
    cmd.set_username_to_activate(userToActivate->text().trimmed().toStdString());
    cmd.set_moderator_name(client->getUserName().toStdString());

    auto *pend = client->prepareModeratorCommand(cmd);
    connect(pend,
            QOverload<const Response &, const CommandContainer &, const QVariant &>::of(&PendingCommand::finished),
            this, &TabAdmin::activateUserProcessResponse);
    client->sendCommand(pend);
}

void TabAdmin::grantReplayAccessProcessResponse(const Response &response, const CommandContainer &, const QVariant &)
{
    auto *event = new Event_ReplayAdded();

    switch (response.response_code()) {
        case Response::RespOk:
            client->replayAddedEventReceived(*event);
            QMessageBox::information(this, tr("Success"), tr("Replay access granted"));
            break;
        case Response::RespContextError:
            QMessageBox::critical(this, tr("Error"), tr("Unable to grant replay access. Replay ID invalid"));
            break;
        default:
            QMessageBox::critical(this, tr("Error"), tr("Unable to grant replay access. Internal error"));
            break;
    }
}

void TabAdmin::activateUserProcessResponse(const Response &response, const CommandContainer &, const QVariant &)
{
    switch (response.response_code()) {
        case Response::RespActivationAccepted:
            QMessageBox::information(this, tr("Success"), tr("User successfully activated"));
            break;
        case Response::RespNameNotFound:
            QMessageBox::critical(this, tr("Error"), tr("Unable to activate user. Username invalid"));
            break;
        case Response::RespActivationFailed:
            QMessageBox::critical(this, tr("Error"), tr("Unable to activate user. User already active"));
            break;
        default:
            QMessageBox::critical(this, tr("Error"), tr("Unable to activate user. Internal error"));
            break;
    }
}

void TabAdmin::actUnlock()
{
    if (fullAdmin)
        adminGroupBox->setEnabled(true);
    lockButton->setEnabled(true);
    unlockButton->setEnabled(false);
    locked = false;
    emit adminLockChanged(false);
}

void TabAdmin::actLock()
{
    if (fullAdmin)
        adminGroupBox->setEnabled(false);
    lockButton->setEnabled(false);
    unlockButton->setEnabled(true);
    locked = true;
    emit adminLockChanged(true);
}
