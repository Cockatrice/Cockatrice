#include "dlg_login_prompt.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

DlgLoginPrompt::DlgLoginPrompt(const QString &serverText, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Sign in"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(
        new QLabel(tr("This link requires you to be signed in.\nSign in to %1:").arg(serverText), this));

    auto *formLayout = new QFormLayout;
    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(tr("Username:"), usernameEdit);
    formLayout->addRow(tr("Password:"), passwordEdit);
    mainLayout->addLayout(formLayout);

    savePasswordCheckBox = new QCheckBox(tr("Save password for this server"), this);
    mainLayout->addWidget(savePasswordCheckBox);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    usernameEdit->setFocus();
}

QString DlgLoginPrompt::username() const
{
    return usernameEdit->text().trimmed();
}

QString DlgLoginPrompt::password() const
{
    return passwordEdit->text();
}

bool DlgLoginPrompt::savePassword() const
{
    return savePasswordCheckBox->isChecked();
}