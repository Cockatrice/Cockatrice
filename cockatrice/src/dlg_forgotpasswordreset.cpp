#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_forgotpasswordreset.h"
#include "settingscache.h"

DlgForgotPasswordReset::DlgForgotPasswordReset(QWidget *parent)
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
        QMessageBox::warning(this, tr("Forgot Password Reset Warning"), tr("Opps, looks like something has gone wrong.  Please re-start the forgot password process by using the forgot password button on the connection screen."));
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
    grid->addWidget(tokenLabel, 3, 0);
    grid->addWidget(tokenEdit, 3, 1);
    grid->addWidget(newpasswordLabel, 4, 0);
    grid->addWidget(newpasswordEdit, 4, 1);
    grid->addWidget(newpasswordverifyLabel, 5, 0);
    grid->addWidget(newpasswordverifyEdit, 5, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));

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
    if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The player name can't be empty."));
        return;
    }

    if (tokenEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The token can't be empty."));
        return;
    }

    if (newpasswordEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The new password can't be empty."));
        return;
    }

    if (newpasswordEdit->text() != newpasswordverifyEdit->text())
    {
        QMessageBox::critical(this, tr("Forgot Password Reset Warning"), tr("The passwords do not match."));
        return;
    }

    settingsCache->servers().setFPHostName(hostEdit->text());
    settingsCache->servers().setFPPort(portEdit->text());
    settingsCache->servers().setFPPlayerName(playernameEdit->text());

    accept();
}

void DlgForgotPasswordReset::actCancel()
{
    reject();
}
