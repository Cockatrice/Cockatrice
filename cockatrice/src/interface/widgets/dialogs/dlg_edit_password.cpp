#include "dlg_edit_password.h"

#include "../../../client/settings/cache_settings.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <libcockatrice/utility/trice_limits.h>

DlgEditPassword::DlgEditPassword(QWidget *parent) : QDialog(parent)
{
    oldPasswordLabel = new QLabel(tr("Old password:"));
    oldPasswordEdit = new QLineEdit();
    oldPasswordEdit->setMaxLength(MAX_NAME_LENGTH);

    auto &servers = SettingsCache::instance().servers();
    if (servers.getSavePassword()) {
        oldPasswordEdit->setText(servers.getPassword());
    }

    oldPasswordLabel->setBuddy(oldPasswordEdit);
    oldPasswordEdit->setEchoMode(QLineEdit::Password);

    newPasswordLabel = new QLabel(tr("New password:"));
    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setMaxLength(MAX_NAME_LENGTH);
    newPasswordLabel->setBuddy(newPasswordLabel);
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    newPasswordLabel2 = new QLabel(tr("Confirm new password:"));
    newPasswordEdit2 = new QLineEdit();
    newPasswordEdit2->setMaxLength(MAX_NAME_LENGTH);
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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgEditPassword::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgEditPassword::reject);

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
    //! \todo this stuff should be using qvalidators
    if (newPasswordEdit->text().length() < 8) {
        QMessageBox::critical(this, tr("Error"), tr("Your password is too short."));
        return;
    } else if (newPasswordEdit->text() != newPasswordEdit2->text()) {
        QMessageBox::warning(this, tr("Error"), tr("The new passwords don't match."));
        return;
    }

    accept();
}
