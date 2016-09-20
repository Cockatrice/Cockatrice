#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <iostream>
#include <QGroupBox>
#include "dlg_connect.h"
#include "settingscache.h"

DlgConnect::DlgConnect(QWidget *parent)
    : QDialog(parent)
{
    previousHostButton = new QRadioButton(tr("Previous Host"), this);
    previousHosts = new QComboBox(this);
    previousHosts->installEventFilter(new DeleteHighlightedItemWhenShiftDelPressedEventFilter);

    QStringList previousHostList = settingsCache->servers().getPreviousHostList();
    if (previousHostList.isEmpty()) {
        previousHostList << "cockatrice.woogerworks.com";
        previousHostList << "vps.poixen.com";
        previousHostList << "chickatrice.net";
        previousHostList << "mtg.tetrarch.co";
    }
    previousHosts->addItems(previousHostList);
    previousHosts->setCurrentIndex(settingsCache->servers().getPrevioushostindex());

    newHostButton = new QRadioButton(tr("New Host"), this);
    
    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit();
    hostEdit->setPlaceholderText(tr("Enter host name"));
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(settingsCache->servers().getPort("4747"));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(settingsCache->servers().getPlayerName("Player"));
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit(settingsCache->servers().getPassword());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    savePasswordCheckBox = new QCheckBox(tr("&Save password"));
    savePasswordCheckBox->setChecked(settingsCache->servers().getSavePassword());

    autoConnectCheckBox = new QCheckBox(tr("A&uto connect"));
    autoConnectCheckBox->setToolTip(tr("Automatically connect to the most recent login when Cockatrice opens"));

    if(savePasswordCheckBox->isChecked())
    {
        autoConnectCheckBox->setChecked(settingsCache->servers().getAutoConnect());
        autoConnectCheckBox->setEnabled(true);
    } else {
        settingsCache->servers().setAutoConnect(0);
        autoConnectCheckBox->setChecked(0);
        autoConnectCheckBox->setEnabled(false);
    }

    connect(savePasswordCheckBox, SIGNAL(stateChanged(int)), this, SLOT(passwordSaved(int)));

    QGridLayout *connectionLayout = new QGridLayout;
    connectionLayout->addWidget(previousHostButton, 0, 1);
    connectionLayout->addWidget(previousHosts, 1, 1);
    connectionLayout->addWidget(newHostButton, 2, 1);
    connectionLayout->addWidget(hostLabel, 3, 0);
    connectionLayout->addWidget(hostEdit, 3, 1);
    connectionLayout->addWidget(portLabel, 4, 0);
    connectionLayout->addWidget(portEdit, 4, 1);
    connectionLayout->addWidget(autoConnectCheckBox, 5, 1);

    QGroupBox *restrictionsGroupBox = new QGroupBox(tr("Server"));
    restrictionsGroupBox->setLayout(connectionLayout);

    QGridLayout *loginLayout = new QGridLayout;
    loginLayout->addWidget(playernameLabel, 0, 0);
    loginLayout->addWidget(playernameEdit, 0, 1);
    loginLayout->addWidget(passwordLabel, 1, 0);
    loginLayout->addWidget(passwordEdit, 1, 1);
    loginLayout->addWidget(savePasswordCheckBox, 2, 1);

    QGroupBox *loginGroupBox = new QGroupBox(tr("Login"));
    loginGroupBox->setLayout(loginLayout);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(restrictionsGroupBox, 0, 0);
    grid->addWidget(loginGroupBox, 1, 0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));
         
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);

    connect(previousHostButton, SIGNAL(toggled(bool)), this, SLOT(previousHostSelected(bool)));
    connect(newHostButton, SIGNAL(toggled(bool)), this, SLOT(newHostSelected(bool)));

    if (settingsCache->servers().getPreviousHostLogin())
        previousHostButton->setChecked(true);
    else
        newHostButton->setChecked(true);

    playernameEdit->setFocus();
}


void DlgConnect::previousHostSelected(bool state) {
    if (state) {
        hostLabel->setDisabled(true);
        hostEdit->setDisabled(true);
        previousHosts->setDisabled(false);
    }
}

void DlgConnect::newHostSelected(bool state) {
    if (state) {
        hostEdit->setDisabled(false);
        hostLabel->setDisabled(false);
        previousHosts->setDisabled(true);
    }
}


void DlgConnect::passwordSaved(int state)
{
    Q_UNUSED(state);
    if(savePasswordCheckBox->isChecked()) {
       autoConnectCheckBox->setEnabled(true);
    } else {
        autoConnectCheckBox->setChecked(0);
        autoConnectCheckBox->setEnabled(false);
    }
}

void DlgConnect::actOk()
{
    settingsCache->servers().setPort(portEdit->text());
    settingsCache->servers().setPlayerName(playernameEdit->text());
    settingsCache->servers().setPassword(savePasswordCheckBox->isChecked() ? passwordEdit->text() : QString());
    settingsCache->servers().setSavePassword(savePasswordCheckBox->isChecked() ? 1 : 0);
    settingsCache->servers().setAutoConnect(autoConnectCheckBox->isChecked() ? 1 : 0);
    settingsCache->servers().setPreviousHostLogin(previousHostButton->isChecked() ? 1 : 0);
    
    QStringList hostList;
    if (newHostButton->isChecked())
        if (!hostEdit->text().trimmed().isEmpty())
            hostList << hostEdit->text();
    
    for (int i = 0; i < previousHosts->count(); i++)
        if(!previousHosts->itemText(i).trimmed().isEmpty())
            hostList << previousHosts->itemText(i);
    
    settingsCache->servers().setPreviousHostList(hostList);
    settingsCache->servers().setPrevioushostindex(previousHosts->currentIndex());

    if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Connect Warning"), tr("The player name can't be empty."));
        return;
    }

    accept();
}


QString DlgConnect::getHost() const {
    return previousHostButton->isChecked() ? previousHosts->currentText() : hostEdit->text();
}

void DlgConnect::actCancel()
{
    settingsCache->servers().setSavePassword(savePasswordCheckBox->isChecked() ? 1 : 0);
    settingsCache->servers().setAutoConnect( autoConnectCheckBox->isChecked() ? 1 : 0);
    reject();
}


bool DeleteHighlightedItemWhenShiftDelPressedEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete) {
            QComboBox* combobox = reinterpret_cast<QComboBox *>(obj);
            combobox->removeItem(combobox->currentIndex());
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}
