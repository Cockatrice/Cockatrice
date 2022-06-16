#include "dlg_forgotpasswordchallenge.h"

#include "stringsizes.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

DlgForgotPasswordChallenge::DlgForgotPasswordChallenge(QWidget *parent) : QDialog(parent)
{
    infoLabel = new QLabel(tr("Enter the information of the account you'd like to request a new password for."));
    infoLabel->setWordWrap(true);

    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailEdit->setMaxLength(MAX_NAME_LENGTH);
    emailLabel->setBuddy(emailLabel);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(infoLabel, 0, 0, 1, 2);
    grid->addWidget(emailLabel, 1, 0);
    grid->addWidget(emailEdit, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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

    accept();
}
