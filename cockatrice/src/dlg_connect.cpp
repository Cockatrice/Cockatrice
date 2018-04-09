#include "dlg_connect.h"
#include "settingscache.h"
#include "userconnection_information.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

#define PUBLIC_SERVERS_URL "https://github.com/Cockatrice/Cockatrice/wiki/Public-Servers"

DlgConnect::DlgConnect(QWidget *parent) : QDialog(parent)
{
    previousHostButton = new QRadioButton(tr("Known Hosts"), this);
    previousHosts = new QComboBox(this);
    previousHosts->installEventFilter(new DeleteHighlightedItemWhenShiftDelPressedEventFilter);

    hps = new HandlePublicServers(this);
    btnRefreshServers = new QPushButton(this);
    btnRefreshServers->setIcon(QPixmap("theme:icons/update"));
    btnRefreshServers->setToolTip(tr("Refresh the server list with known public servers"));
    btnRefreshServers->setFixedWidth(50);

    connect(hps, SIGNAL(sigPublicServersDownloadedSuccessfully()), this, SLOT(rebuildComboBoxList()));
    connect(hps, SIGNAL(sigPublicServersDownloadedUnsuccessfully(int)), this, SLOT(rebuildComboBoxList(int)));
    connect(btnRefreshServers, SIGNAL(released()), this, SLOT(downloadThePublicServers()));

    connect(this, SIGNAL(sigPublicServersDownloaded()), this, SLOT(rebuildComboBoxList()));
    preRebuildComboBoxList();

    newHostButton = new QRadioButton(tr("New Host"), this);

    saveLabel = new QLabel(tr("Name:"));
    saveEdit = new QLineEdit(settingsCache->servers().getSaveName());
    saveLabel->setBuddy(saveEdit);

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settingsCache->servers().getHostname());
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

    if (savePasswordCheckBox->isChecked()) {
        autoConnectCheckBox->setChecked(static_cast<bool>(settingsCache->servers().getAutoConnect()));
        autoConnectCheckBox->setEnabled(true);
    } else {
        settingsCache->servers().setAutoConnect(0);
        autoConnectCheckBox->setChecked(false);
        autoConnectCheckBox->setEnabled(false);
    }

    connect(savePasswordCheckBox, SIGNAL(stateChanged(int)), this, SLOT(passwordSaved(int)));

    btnForgotPassword = new QPushButton(tr("Forgot password"));
    connect(btnForgotPassword, SIGNAL(released()), this, SLOT(actForgotPassword()));

    btnOk = new QPushButton(tr("Connect"));
    btnOk->setFixedWidth(100);
    connect(btnOk, SIGNAL(released()), this, SLOT(actOk()));

    btnCancel = new QPushButton(tr("Cancel"));
    btnCancel->setFixedWidth(100);
    connect(btnCancel, SIGNAL(released()), this, SLOT(actCancel()));

    newHolderLayout = new QHBoxLayout;
    newHolderLayout->addWidget(previousHosts);
    newHolderLayout->addWidget(btnRefreshServers);

    connectionLayout = new QGridLayout;
    connectionLayout->addWidget(previousHostButton, 0, 1);
    connectionLayout->addLayout(newHolderLayout, 1, 1, 1, 2);
    connectionLayout->addWidget(newHostButton, 2, 1);
    connectionLayout->addWidget(saveLabel, 3, 0);
    connectionLayout->addWidget(saveEdit, 3, 1);
    connectionLayout->addWidget(hostLabel, 4, 0);
    connectionLayout->addWidget(hostEdit, 4, 1);
    connectionLayout->addWidget(portLabel, 5, 0);
    connectionLayout->addWidget(portEdit, 5, 1);
    connectionLayout->addWidget(autoConnectCheckBox, 6, 1);

    buttons = new QGridLayout;
    buttons->addWidget(btnOk, 0, 0);
    buttons->addWidget(btnForgotPassword, 0, 1);
    buttons->addWidget(btnCancel, 0, 2);

    restrictionsGroupBox = new QGroupBox(tr("Server"));
    restrictionsGroupBox->setLayout(connectionLayout);

    loginLayout = new QGridLayout;
    loginLayout->addWidget(playernameLabel, 0, 0);
    loginLayout->addWidget(playernameEdit, 0, 1);
    loginLayout->addWidget(passwordLabel, 1, 0);
    loginLayout->addWidget(passwordEdit, 1, 1);
    loginLayout->addWidget(savePasswordCheckBox, 2, 1);

    loginGroupBox = new QGroupBox(tr("Login"));
    loginGroupBox->setLayout(loginLayout);

    btnGroupBox = new QGroupBox(tr(""));
    btnGroupBox->setLayout(buttons);

    grid = new QGridLayout;
    grid->addWidget(restrictionsGroupBox, 0, 0);
    grid->addWidget(loginGroupBox, 1, 0);
    grid->addWidget(btnGroupBox, 2, 0);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);

    connect(previousHostButton, SIGNAL(toggled(bool)), this, SLOT(previousHostSelected(bool)));
    connect(newHostButton, SIGNAL(toggled(bool)), this, SLOT(newHostSelected(bool)));

    previousHostButton->setChecked(true);

    connect(previousHosts, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(updateDisplayInfo(const QString &)));

    playernameEdit->setFocus();
}

DlgConnect::~DlgConnect() = default;

void DlgConnect::actSaveConfig()
{
    bool updateSuccess = settingsCache->servers().updateExistingServer(
        saveEdit->text().trimmed(), hostEdit->text().trimmed(), portEdit->text().trimmed(),
        playernameEdit->text().trimmed(), passwordEdit->text(), savePasswordCheckBox->isChecked());

    if (!updateSuccess) {
        settingsCache->servers().addNewServer(saveEdit->text().trimmed(), hostEdit->text().trimmed(),
                                              portEdit->text().trimmed(), playernameEdit->text().trimmed(),
                                              passwordEdit->text(), savePasswordCheckBox->isChecked());
    }

    preRebuildComboBoxList();
}

void DlgConnect::downloadThePublicServers()
{
    btnRefreshServers->setDisabled(true);
    previousHosts->clear();
    previousHosts->addItem(placeHolderText);
    hps->downloadPublicServers();
}

void DlgConnect::preRebuildComboBoxList()
{
    UserConnection_Information uci;
    savedHostList = uci.getServerInfo();

    if (savedHostList.size() == 1) {
        downloadThePublicServers();
    } else {
        rebuildComboBoxList();
    }
}

void DlgConnect::rebuildComboBoxList(int failure)
{
    Q_UNUSED(failure);

    previousHosts->clear();

    UserConnection_Information uci;
    savedHostList = uci.getServerInfo();

    int i = 0;
    for (auto pair : savedHostList) {
        auto tmp = pair.second;
        QString saveName = tmp.getSaveName();
        if (saveName.size()) {
            previousHosts->addItem(saveName);

            if (settingsCache->servers().getPrevioushostName() == saveName) {
                previousHosts->setCurrentIndex(i);
            }

            i++;
        }
    }

    btnRefreshServers->setDisabled(false);
}

void DlgConnect::previousHostSelected(bool state)
{
    if (state) {
        saveEdit->setDisabled(true);
        previousHosts->setDisabled(false);
        btnRefreshServers->setDisabled(false);
    }
}

void DlgConnect::updateDisplayInfo(const QString &saveName)
{
    if (saveEdit == nullptr || saveName == placeHolderText) {
        return;
    }

    UserConnection_Information uci;
    QStringList data = uci.getServerInfo(saveName);

    bool savePasswordStatus = (data.at(5) == "1");

    saveEdit->setText(data.at(0));
    hostEdit->setText(data.at(1));
    portEdit->setText(data.at(2));
    playernameEdit->setText(data.at(3));
    savePasswordCheckBox->setChecked(savePasswordStatus);

    if (savePasswordStatus) {
        passwordEdit->setText(data.at(4));
    }
}

void DlgConnect::newHostSelected(bool state)
{
    if (state) {
        previousHosts->setDisabled(true);
        btnRefreshServers->setDisabled(true);
        hostEdit->clear();
        portEdit->clear();
        playernameEdit->clear();
        passwordEdit->clear();
        savePasswordCheckBox->setChecked(false);
        saveEdit->clear();
        saveEdit->setPlaceholderText("New Menu Name");
        saveEdit->setDisabled(false);
    } else {
        preRebuildComboBoxList();
    }
}

void DlgConnect::passwordSaved(int state)
{
    Q_UNUSED(state);
    if (savePasswordCheckBox->isChecked()) {
        autoConnectCheckBox->setEnabled(true);
    } else {
        autoConnectCheckBox->setChecked(false);
        autoConnectCheckBox->setEnabled(false);
    }
}

void DlgConnect::actOk()
{
    if (newHostButton->isChecked()) {
        if (saveEdit->text().isEmpty()) {
            QMessageBox::critical(this, tr("Connection Warning"), tr("You need to name your new connection profile."));
            return;
        }

        settingsCache->servers().addNewServer(saveEdit->text().trimmed(), hostEdit->text().trimmed(),
                                              portEdit->text().trimmed(), playernameEdit->text().trimmed(),
                                              passwordEdit->text(), savePasswordCheckBox->isChecked());
    } else {
        settingsCache->servers().updateExistingServer(saveEdit->text().trimmed(), hostEdit->text().trimmed(),
                                                      portEdit->text().trimmed(), playernameEdit->text().trimmed(),
                                                      passwordEdit->text(), savePasswordCheckBox->isChecked());
    }

    settingsCache->servers().setPrevioushostName(saveEdit->text());
    settingsCache->servers().setAutoConnect(autoConnectCheckBox->isChecked());

    if (playernameEdit->text().isEmpty()) {
        QMessageBox::critical(this, tr("Connect Warning"), tr("The player name can't be empty."));
        return;
    }

    accept();
}

QString DlgConnect::getHost() const
{
    return hostEdit->text().trimmed();
}

void DlgConnect::actCancel()
{
    settingsCache->servers().setSavePassword(savePasswordCheckBox->isChecked());
    settingsCache->servers().setAutoConnect(autoConnectCheckBox->isChecked());
    reject();
}

bool DeleteHighlightedItemWhenShiftDelPressedEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Delete) {
            auto *combobox = reinterpret_cast<QComboBox *>(obj);
            combobox->removeItem(combobox->currentIndex());
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

void DlgConnect::actForgotPassword()
{
    emit sigStartForgotPasswordRequest();
    reject();
}