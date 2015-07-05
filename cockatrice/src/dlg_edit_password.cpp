#include <QSettings>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>

#include "dlg_edit_password.h"

DlgEditPassword::DlgEditPassword(QWidget *parent)
    : QDialog(parent)
{
    QSettings settings;
    settings.beginGroup("server");

    oldPasswordLabel = new QLabel(tr("Old password:"));
    oldPasswordEdit = new QLineEdit(settings.value("password").toString());
    oldPasswordLabel->setBuddy(oldPasswordEdit);
    oldPasswordEdit->setEchoMode(QLineEdit::Password);

    newPasswordLabel = new QLabel(tr("New password:"));
    newPasswordEdit = new QLineEdit();
    newPasswordLabel->setBuddy(newPasswordLabel);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(oldPasswordLabel, 0, 0);
    grid->addWidget(oldPasswordEdit, 0, 1);
    grid->addWidget(newPasswordLabel, 1, 0);
    grid->addWidget(newPasswordEdit, 1, 1);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));
         
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
    QSettings settings;
    settings.beginGroup("server");
    // always save the password so it will be picked up by the connect dialog
    settings.setValue("password", newPasswordEdit->text());
    settings.endGroup();
  
    accept();
}

void DlgEditPassword::actCancel()
{
    reject();
}
