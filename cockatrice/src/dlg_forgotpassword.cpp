#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_forgotpassword.h"

DlgForgotPassword::DlgForgotPassword(QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("Host:"));
    hostEdit = new QLineEdit();

    portLabel = new QLabel(tr("Port:"));
    portEdit = new QLineEdit();

    playernameLabel = new QLabel(tr("Player Name:"));
    playernameEdit = new QLineEdit();

    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();

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

    setWindowTitle(tr("Forgot password"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPassword::actOk()
{
    if (emailEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Warning"), tr("Your email address can't be empty."));
        return;
    }
    if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Warning"), tr("The player name can't be empty."));
        return;
    }
    accept();
}

void DlgForgotPassword::actCancel()
{
    reject();
}
