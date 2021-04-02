#include "dlg_edit_password.h"

#include "settingscache.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

DlgEditPassword::DlgEditPassword(QWidget *parent) : QDialog(parent)
{
    oldPasswordLabel = new QLabel(tr("Old password:"));
    oldPasswordEdit = new QLineEdit();

    auto &servers = SettingsCache::instance().servers();
    if (servers.getSavePassword()) {
        oldPasswordEdit->setText(servers.getPassword());
    }

    oldPasswordLabel->setBuddy(oldPasswordEdit);
    oldPasswordEdit->setEchoMode(QLineEdit::Password);

    newPasswordLabel = new QLabel(tr("New password:"));
    newPasswordEdit = new QLineEdit();
    newPasswordLabel->setBuddy(newPasswordLabel);
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    newPasswordLabel2 = new QLabel(tr("Confirm new password:"));
    newPasswordEdit2 = new QLineEdit();
    newPasswordLabel2->setBuddy(newPasswordLabel2);
    newPasswordEdit2->setEchoMode(QLineEdit::Password);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(oldPasswordLabel, 0, 0);
    grid->addWidget(oldPasswordEdit, 0, 1);
    grid->addWidget(newPasswordLabel, 1, 0);
    grid->addWidget(newPasswordEdit, 1, 1);
    grid->addWidget(newPasswordLabel2, 2, 0);
    grid->addWidget(newPasswordEdit2, 2, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Change password"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgEditPassword::actOk()
{
    if (newPasswordEdit->text() != newPasswordEdit2->text()) {
        QMessageBox::warning(this, tr("Error"), tr("The new passwords don't match."));
        return;
    }

    accept();
}
