#include "dlg_connect.h"

#include "../../../client/settings/cache_settings.h"

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
#include <libcockatrice/utility/trice_limits.h>

DlgConnect::DlgConnect(QWidget *parent) : QDialog(parent)
{
    previousHostButton = new QRadioButton(tr("Known Hosts"), this);
    previousHosts = new QComboBox(this);

    btnDeleteServer = new QPushButton(this);
    btnDeleteServer->setIcon(QPixmap("theme:icons/remove_row"));
    btnDeleteServer->setToolTip(tr("Delete the currently selected saved server"));
    btnDeleteServer->setFixedWidth(30);

    connect(btnDeleteServer, &QPushButton::clicked, this, &DlgConnect::actRemoveSavedServer);

    hps = new HandlePublicServers(this);
    btnRefreshServers = new QPushButton(this);
    btnRefreshServers->setIcon(QPixmap("theme:icons/sync"));
    btnRefreshServers->setToolTip(tr("Refresh the server list with known public servers"));
    btnRefreshServers->setFixedWidth(30);

    connect(hps, &HandlePublicServers::sigPublicServersDownloadedSuccessfully, this, [this] { rebuildComboBoxList(); });
    connect(hps, &HandlePublicServers::sigPublicServersDownloadedUnsuccessfully, this,
            &DlgConnect::rebuildComboBoxList);
    connect(btnRefreshServers, &QPushButton::released, this, &DlgConnect::downloadThePublicServers);

    connect(this, &DlgConnect::sigPublicServersDownloaded, this, [this] { rebuildComboBoxList(); });
    preRebuildComboBoxList();

    newHostButton = new QRadioButton(tr("New Host"), this);

    saveLabel = new QLabel(tr("Name:"));
    saveEdit = new QLineEdit;
    saveEdit->setMaxLength(MAX_NAME_LENGTH);
    saveLabel->setBuddy(saveEdit);

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit;
    hostEdit->setMaxLength(MAX_NAME_LENGTH);
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit;
    portEdit->setValidator(new QIntValidator(0, 0xffff, portEdit));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit;
    playernameEdit->setMaxLength(MAX_NAME_LENGTH);
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit;
    passwordEdit->setMaxLength(MAX_NAME_LENGTH);
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);

    savePasswordCheckBox = new QCheckBox(tr("&Save password"));

    autoConnectCheckBox = new QCheckBox(tr("A&uto connect"));
    autoConnectCheckBox->setToolTip(tr("Automatically connect to the most recent login when Cockatrice opens"));

    auto &servers = SettingsCache::instance().servers();
    if (servers.getSavePassword()) {
        autoConnectCheckBox->setChecked(servers.getAutoConnect() > 0);
        autoConnectCheckBox->setEnabled(true);
    } else {
        servers.setAutoConnect(0);
        autoConnectCheckBox->setChecked(false);
        autoConnectCheckBox->setEnabled(false);
    }

    connect(savePasswordCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &DlgConnect::passwordSaved);
    connect(autoConnectCheckBox, &QCheckBox::QT_STATE_CHANGED, &servers, &ServersSettings::setAutoConnect);

    serverIssuesLabel =
        new QLabel(tr("If you have any trouble connecting or registering then contact the server staff for help!"));
    serverIssuesLabel->setWordWrap(true);
    serverContactLabel = new QLabel(tr("Webpage") + ":");
    serverContactLink = new QLabel;
    serverContactLink->setTextFormat(Qt::RichText);
    serverContactLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    serverContactLink->setOpenExternalLinks(true);

    updateDisplayInfo(previousHosts->currentText());

    btnForgotPassword = new QPushButton(this);
    btnForgotPassword->setIcon(QPixmap("theme:icons/forgot_password"));
    btnForgotPassword->setToolTip(tr("Reset Password"));
    btnForgotPassword->setFixedWidth(30);
    connect(btnForgotPassword, &QPushButton::released, this, &DlgConnect::actForgotPassword);

    forgotPasswordLabel = new QLabel(tr("Forgot password?"));
    forgotPasswordLabel->setBuddy(btnForgotPassword);

    btnConnect = new QPushButton(tr("&Connect"));
    connect(btnConnect, &QPushButton::released, this, &DlgConnect::actOk);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(btnConnect, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgConnect::reject);

    newHolderLayout = new QHBoxLayout;
    newHolderLayout->addWidget(previousHosts);
    newHolderLayout->addWidget(btnDeleteServer);
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

    restrictionsGroupBox = new QGroupBox(tr("Server"));
    restrictionsGroupBox->setLayout(connectionLayout);

    serverInfoLayout = new QGridLayout;
    serverInfoLayout->addWidget(serverIssuesLabel, 0, 0, 1, 4, Qt::AlignTop);
    serverInfoLayout->addWidget(serverContactLabel, 1, 0);
    serverInfoLayout->addWidget(serverContactLink, 1, 1, 1, 3);

    forgotPasswordLayout = new QHBoxLayout;
    forgotPasswordLayout->addWidget(forgotPasswordLabel, 0, Qt::AlignLeft);
    forgotPasswordLayout->addWidget(btnForgotPassword, 0, Qt::AlignLeft);

    loginLayout = new QGridLayout;
    loginLayout->addWidget(playernameLabel, 0, 0);
    loginLayout->addWidget(playernameEdit, 0, 1, 1, 2);
    loginLayout->addWidget(passwordLabel, 1, 0);
    loginLayout->addWidget(passwordEdit, 1, 1, 1, 2);
    loginLayout->addWidget(savePasswordCheckBox, 2, 1);
    loginLayout->addLayout(forgotPasswordLayout, 3, 1);

    loginGroupBox = new QGroupBox(tr("Login"));
    loginGroupBox->setLayout(loginLayout);

    serverInfoGroupBox = new QGroupBox(tr("Server Contact"));
    serverInfoGroupBox->setLayout(serverInfoLayout);

    grid = new QGridLayout;
    grid->addWidget(restrictionsGroupBox, 0, 0);
    grid->addWidget(serverInfoGroupBox, 1, 0);
    grid->addWidget(loginGroupBox, 2, 0);

    mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Connect to Server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);

    connect(previousHostButton, &QRadioButton::toggled, this, &DlgConnect::previousHostSelected);
    connect(newHostButton, &QRadioButton::toggled, this, &DlgConnect::newHostSelected);

    previousHostButton->setChecked(true);

    connect(previousHosts, &QComboBox::currentTextChanged, this, &DlgConnect::updateDisplayInfo);

    playernameEdit->setFocus();
}

DlgConnect::~DlgConnect() = default;

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

    auto &servers = SettingsCache::instance().servers();
    bool autoConnectEnabled = servers.getAutoConnect() > 0;
    QString previousHostName = servers.getPrevioushostName();
    QString autoConnectSaveName = servers.getSaveName();

    int index = 0;

    for (const auto &pair : savedHostList) {
        const auto &tmp = pair.second;
        QString saveName = tmp.getSaveName();
        if (saveName.size()) {
            previousHosts->addItem(saveName);

            if (autoConnectEnabled) {
                if (saveName.compare(autoConnectSaveName) == 0) {
                    previousHosts->setCurrentIndex(index);
                }
            } else if (saveName.compare(previousHostName) == 0) {
                previousHosts->setCurrentIndex(index);
            }

            ++index;
        }
    }

    // Re-enable the refresh server button
    btnRefreshServers->setDisabled(false);
}

void DlgConnect::previousHostSelected(bool state)
{
    if (state) {
        saveEdit->setDisabled(true);
        hostEdit->setDisabled(true);
        portEdit->setDisabled(true);
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
    QStringList _data = uci.getServerInfo(saveName);

    if (_data.isEmpty()) {
        _data << ""
              << ""
              << ""
              << ""
              << ""
              << ""
              << "";
    }

    bool savePasswordStatus = (_data.at(5) == "1");

    saveEdit->setText(_data.at(0));
    hostEdit->setText(_data.at(1));
    portEdit->setText(_data.at(2));
    playernameEdit->setText(_data.at(3));
    savePasswordCheckBox->setChecked(savePasswordStatus);

    if (savePasswordStatus) {
        passwordEdit->setText(_data.at(4));
    }

    if (!_data.at(6).isEmpty()) {
        QString formattedLink = "<a href=\"" + _data.at(6) + "\">" + _data.at(6) + "</a>";
        serverContactLabel->setText(tr("Webpage") + ":");
        serverContactLink->setText(formattedLink);
    } else {
        serverContactLabel->setText("");
        serverContactLink->setText("");
    }
}

void DlgConnect::newHostSelected(bool state)
{
    if (state) {
        previousHosts->setDisabled(true);
        btnRefreshServers->setDisabled(true);
        hostEdit->clear();
        hostEdit->setPlaceholderText(tr("Server URL"));
        hostEdit->setDisabled(false);
        portEdit->clear();
        portEdit->setPlaceholderText(tr("Communication Port"));
        portEdit->setDisabled(false);
        playernameEdit->clear();
        passwordEdit->clear();
        saveEdit->clear();
        saveEdit->setPlaceholderText(tr("Unique Server Name"));
        saveEdit->setDisabled(false);
        serverContactLabel->setText("");
        serverContactLink->setText("");
    } else {
        preRebuildComboBoxList();
    }
}

void DlgConnect::passwordSaved(QT_STATE_CHANGED_T state)
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
    ServersSettings &servers = SettingsCache::instance().servers();

    if (newHostButton->isChecked()) {
        if (saveEdit->text().isEmpty()) {
            QMessageBox::critical(this, tr("Connection Warning"), tr("You need to name your new connection profile."));
            return;
        }

        servers.addNewServer(saveEdit->text().trimmed(), hostEdit->text().trimmed(), portEdit->text().trimmed(),
                             playernameEdit->text().trimmed(), passwordEdit->text(), savePasswordCheckBox->isChecked());
    } else {
        servers.updateExistingServer(saveEdit->text().trimmed(), hostEdit->text().trimmed(), portEdit->text().trimmed(),
                                     playernameEdit->text().trimmed(), passwordEdit->text(),
                                     savePasswordCheckBox->isChecked());
    }

    servers.setPrevioushostName(saveEdit->text());

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

void DlgConnect::actForgotPassword()
{
    ServersSettings &servers = SettingsCache::instance().servers();
    servers.setFPHostName(hostEdit->text());
    servers.setFPPort(portEdit->text());
    servers.setFPPlayerName(playernameEdit->text().trimmed());

    emit sigStartForgotPasswordRequest();
}

void DlgConnect::actRemoveSavedServer()
{
    SettingsCache::instance().servers().removeServer(hostEdit->text());
    previousHosts->removeItem(previousHosts->currentIndex());
}