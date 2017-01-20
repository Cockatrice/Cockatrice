#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_forgotpasswordreset.h"

DlgForgotPasswordReset::DlgForgotPasswordReset(QString host, int port, QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("Host:"));
    hostEdit = new QLineEdit(host);
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("Port:"));
    portEdit = new QLineEdit(QString::number(port));
    portLabel->setBuddy(portEdit);

    tokenLabel = new QLabel(tr("Activation Token:"));
    tokenEdit = new QLineEdit();
    
    newpasswordLabel = new QLabel(tr("New Password:"));
    newpasswordEdit = new QLineEdit();
	newpasswordEdit->setEchoMode(QLineEdit::Password);

    confirmnewpasswordLabel = new QLabel(tr("Confirm New Password:"));
    confirmnewpasswordEdit = new QLineEdit();
	confirmnewpasswordEdit->setEchoMode(QLineEdit::Password);
    
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(hostLabel, 0, 0);
	grid->addWidget(hostEdit, 0, 1);
	grid->addWidget(portLabel, 1, 0);
	grid->addWidget(portEdit, 1, 1);
	grid->addWidget(tokenLabel, 2, 0);
	grid->addWidget(tokenEdit, 2, 1);
	grid->addWidget(newpasswordLabel, 3, 0);
	grid->addWidget(newpasswordEdit, 3, 1);
    grid->addWidget(confirmnewpasswordLabel, 4, 0);
    grid->addWidget(confirmnewpasswordEdit, 4, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Reset Forgot Password"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordReset::actOk()
{
	
    if (hostEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Warning"), tr("Host can't be empty."));
        return;
    }
    if (portEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Warning"), tr("Port can't be empty."));
        return;
    }
    if (tokenEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Warning"), tr("Your activation token can't be empty."));
        return;
    }
    if(newpasswordEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Warning"), tr("Your new password can't be empty."));
        return;
    }
    if (newpasswordEdit->text() != confirmnewpasswordEdit->text())
    {
        QMessageBox::critical(this, tr("Warning"), tr("Your passwords do not match."));
        return;
    }
    accept();
}

void DlgForgotPasswordReset::actCancel()
{
    reject();
}
