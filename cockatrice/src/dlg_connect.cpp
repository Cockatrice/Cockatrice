#include <QSettings>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include "dlg_connect.h"

DlgConnect::DlgConnect(QWidget *parent)
    : QDialog(parent)
{
    QSettings settings;
    settings.beginGroup("server");

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settings.value("hostname", "cockatrice.woogerworks.com").toString());
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(settings.value("port", "4747").toString());
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(settings.value("playername", "Player").toString());
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit(settings.value("password").toString());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    savePasswordCheckBox = new QCheckBox(tr("&Save password"));
    savePasswordCheckBox->setChecked(settings.value("save_password", 1).toInt());

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(hostLabel, 0, 0);
    grid->addWidget(hostEdit, 0, 1);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(portEdit, 1, 1);
    grid->addWidget(playernameLabel, 2, 0);
    grid->addWidget(playernameEdit, 2, 1);
    grid->addWidget(passwordLabel, 3, 0);
    grid->addWidget(passwordEdit, 3, 1);
    grid->addWidget(savePasswordCheckBox, 4, 0, 1, 2);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgConnect::actOk()
{
    QSettings settings;
    settings.beginGroup("server");
    settings.setValue("hostname", hostEdit->text());
    settings.setValue("port", portEdit->text());
    settings.setValue("playername", playernameEdit->text());
    settings.setValue("password", savePasswordCheckBox->isChecked() ? passwordEdit->text() : QString());
    settings.setValue("save_password", savePasswordCheckBox->isChecked() ? 1 : 0);
    settings.endGroup();

    accept();
}
