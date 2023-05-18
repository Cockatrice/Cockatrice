#include "dlg_forgotpasswordreset.h"

#include "stringsizes.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

DlgForgotPasswordReset::DlgForgotPasswordReset(QWidget *parent) : QDialog(parent)
{
    infoLabel = new QLabel(tr("Enter the received token and the new password in order to set your new password."));
    infoLabel->setWordWrap(true);

    tokenLabel = new QLabel(tr("Token:"));
    tokenEdit = new QLineEdit();
    tokenEdit->setMaxLength(MAX_NAME_LENGTH);
    tokenLabel->setBuddy(tokenLabel);

    newpasswordLabel = new QLabel(tr("New Password:"));
    newpasswordEdit = new QLineEdit();
    newpasswordEdit->setMaxLength(MAX_NAME_LENGTH);
    newpasswordLabel->setBuddy(newpasswordEdit);
    newpasswordEdit->setEchoMode(QLineEdit::Password);

    newpasswordverifyLabel = new QLabel(tr("New Password:"));
    newpasswordverifyEdit = new QLineEdit();
    newpasswordverifyEdit->setMaxLength(MAX_NAME_LENGTH);
    newpasswordverifyLabel->setBuddy(newpasswordEdit);
    newpasswordverifyEdit->setEchoMode(QLineEdit::Password);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(infoLabel, 0, 0, 1, 2);
    grid->addWidget(tokenLabel, 1, 0);
    grid->addWidget(tokenEdit, 1, 1);
    grid->addWidget(newpasswordLabel, 2, 0);
    grid->addWidget(newpasswordEdit, 2, 1);
    grid->addWidget(newpasswordverifyLabel, 3, 0);
    grid->addWidget(newpasswordverifyEdit, 3, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Reset Password"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgForgotPasswordReset::actOk()
{
    if (tokenEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Reset Password Error"), tr("The token can't be empty."));
        return;
    }

    if (newpasswordEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Reset Password Error"), tr("The new password can't be empty."));
        return;
    }

    // TODO this stuff should be using qvalidators
    if (newpasswordEdit->text().length() < 8) {
        QMessageBox::critical(this, tr("Error"), tr("Your password is too short."));
        return;
    } else if (newpasswordEdit->text() != newpasswordverifyEdit->text()) {
        QMessageBox::critical(this, tr("Reset Password Error"), tr("The passwords do not match."));
        return;
    }

    accept();
}
