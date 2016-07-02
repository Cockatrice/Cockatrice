#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include "tab_admin.h"
#include "abstractclient.h"

#include "pb/admin_commands.pb.h"

ShutdownDialog::ShutdownDialog(QWidget *parent)
    : QDialog(parent)
{
    QLabel *reasonLabel = new QLabel(tr("&Reason for shutdown:"));
    reasonEdit = new QLineEdit;
    reasonLabel->setBuddy(reasonEdit);
    QLabel *minutesLabel = new QLabel(tr("&Time until shutdown (minutes):"));
    minutesEdit = new QSpinBox;
    minutesLabel->setBuddy(minutesEdit);
    minutesEdit->setMinimum(0);
    minutesEdit->setValue(5);
    minutesEdit->setMaximum(999);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
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
    connect(updateServerMessageButton, SIGNAL(clicked()), this, SLOT(actUpdateServerMessage()));
    shutdownServerButton = new QPushButton;
    connect(shutdownServerButton, SIGNAL(clicked()), this, SLOT(actShutdownServer()));
    reloadConfigButton = new QPushButton;
    connect(reloadConfigButton, SIGNAL(clicked()), this, SLOT(actReloadConfig()));
    
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(updateServerMessageButton);
    vbox->addWidget(shutdownServerButton);
    vbox->addWidget(reloadConfigButton);
    vbox->addStretch();
    
    adminGroupBox = new QGroupBox;
    adminGroupBox->setLayout(vbox);
    adminGroupBox->setEnabled(false);
    
    unlockButton = new QPushButton;
    connect(unlockButton, SIGNAL(clicked()), this, SLOT(actUnlock()));
    lockButton = new QPushButton;
    lockButton->setEnabled(false);
    connect(lockButton, SIGNAL(clicked()), this, SLOT(actLock()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(adminGroupBox);
    mainLayout->addWidget(unlockButton);
    mainLayout->addWidget(lockButton);
    
    retranslateUi();

    QWidget * mainWidget = new QWidget(this);
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
        
        client->sendCommand(client->prepareAdminCommand(cmd));
    }
}

void TabAdmin::actReloadConfig()
{
    Command_ReloadConfig cmd;
    client->sendCommand(client->prepareAdminCommand(cmd));
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
