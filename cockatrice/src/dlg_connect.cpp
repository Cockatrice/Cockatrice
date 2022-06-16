#include "dlg_connect.h"

#include "dlg_forgotpasswordchallenge.h"
#include "dlg_forgotpasswordreset.h"
#include "remoteclient.h"
#include "settings/serverssettings.h"
#include "settingscache.h"
#include "stringsizes.h"
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
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

DlgConnect::DlgConnect(QWidget *parent, RemoteClient *_client) : QDialog(parent), mode(Login), client(_client), hps(this)
{
    setupUI();

    auto &servers = SettingsCache::instance().servers();
    currentSaveName = servers.getPreviousHostName();
    autoConnectEnabled = servers.getAutoConnect();
    updateDisplayInfo();
    if (servers.start() == servers.end()) {
        downloadThePublicServers();
    } else {
        rebuildComboBoxList();
    }

    connect(&hps, SIGNAL(sigPublicServersDownloadedSuccessfully()), this, SLOT(rebuildComboBoxList()));
    connect(&hps, SIGNAL(sigPublicServersDownloadedUnsuccessfully(int)), this, SLOT(rebuildComboBoxList(int)));
    connect(btnRefreshServers, SIGNAL(clicked()), this, SLOT(downloadThePublicServers()));
    connect(savePasswordCheckBox, SIGNAL(stateChanged(int)), this, SLOT(passwordSaved(int)));
    connect(previousHosts, SIGNAL(currentIndexChanged(int)), this, SLOT(selectServer(int)));
    connect(btnForgotPassword, SIGNAL(clicked()), this, SLOT(actForgotPassword()));

    connect(btnConnect, SIGNAL(clicked()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

DlgConnect::~DlgConnect() = default;

void DlgConnect::setupUI()
{
    previousHosts = new QComboBox(this);

    btnRefreshServers = new QPushButton(this);
    btnRefreshServers->setIcon(QPixmap("theme:icons/sync"));
    btnRefreshServers->setText(tr("Refresh"));
    btnRefreshServers->setToolTip(tr("Refresh the server list with known public servers"));

    saveLabel = new QLabel(this);
    saveLabel->setText(tr("&Name:"));
    saveEdit = new QLineEdit(this);
    saveEdit->setMaxLength(MAX_NAME_LENGTH);
    saveEdit->setPlaceholderText(tr("Unique Server Name"));
    saveLabel->setBuddy(saveEdit);

    hostLabel = new QLabel(this);
    hostLabel->setText(tr("&Host:"));
    hostEdit = new QLineEdit(this);
    hostEdit->setMaxLength(MAX_NAME_LENGTH);
    hostEdit->setPlaceholderText(tr("Server URL"));
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(this);
    portLabel->setText(tr("&Port:"));
    portEdit = new QLineEdit(this);
    portEdit->setValidator(new QIntValidator(0, 0xffff, portEdit));
    portEdit->setPlaceholderText(tr("Communication Port"));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(this);
    playernameLabel->setText(tr("Player &Name:"));
    playernameEdit = new QLineEdit(this);
    playernameEdit->setMaxLength(MAX_NAME_LENGTH);
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(this);
    passwordLabel->setText(tr("&Password:"));
    passwordEdit = new QLineEdit(this);
    passwordEdit->setMaxLength(MAX_NAME_LENGTH);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLabel->setBuddy(passwordEdit);

    passwordConfirmationLabel = new QLabel(this);
    passwordConfirmationLabel->setText(tr("&Password (again):"));
    passwordConfirmationEdit = new QLineEdit(this);
    passwordConfirmationEdit->setMaxLength(MAX_NAME_LENGTH);
    passwordConfirmationEdit->setEchoMode(QLineEdit::Password);
    passwordConfirmationLabel->setBuddy(passwordConfirmationEdit);

    savePasswordCheckBox = new QCheckBox(this);
    savePasswordCheckBox->setText(tr("&Save password"));

    btnForgotPassword = new QPushButton(this);
    btnForgotPassword->setText(tr("&Reset Password"));
    btnForgotPassword->setIcon(QPixmap("theme:icons/forgot_password"));

    autoConnectCheckBox = new QCheckBox(this);
    autoConnectCheckBox->setText(tr("&Auto connect"));
    autoConnectCheckBox->setToolTip(tr("Automatically connect to the most recent login when Cockatrice opens"));

    serverIssuesLabel = new QLabel(this);
    serverIssuesLabel->setText(
        tr("If you have any trouble connecting or registering then contact the server staff for help!"));
    serverIssuesLabel->setWordWrap(true);
    serverContactLabel = new QLabel(this);
    serverContactLabel->setText(tr("Webpage:"));
    serverContactLink = new QLabel(this);
    serverContactLink->setTextFormat(Qt::RichText);
    serverContactLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    serverContactLink->setOpenExternalLinks(true);

    realNameLabel = new QLabel(this);
    realNameLabel->setText(tr("&Real Name:"));
    realNameEdit = new QLineEdit(this);
    realNameEdit->setMaxLength(MAX_NAME_LENGTH);
    realNameLabel->setBuddy(realNameEdit);

    countryLabel = new QLabel(this);
    countryLabel->setText(tr("&Country:"));
    countryEdit = new QComboBox(this);
    countryEdit->addItem(tr("Undefined"));
    QStringList countries = SettingsCache::instance().getCountries();
    for (const auto &code : countries) {
        countryEdit->addItem(QPixmap("theme:countries/" + code), code);
    }
    countryEdit->setCurrentIndex(0);
    countryLabel->setBuddy(countryEdit);

    emailLabel = new QLabel(this);
    emailLabel->setText(tr("&Email:"));
    emailEdit = new QLineEdit(this);
    emailEdit->setMaxLength(MAX_NAME_LENGTH);
    emailLabel->setBuddy(emailEdit);

    btnConnect = new QPushButton(this);
    btnConnect->setText(tr("&Connect"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    buttonBox->addButton(btnConnect, QDialogButtonBox::AcceptRole);

    auto *newHolderLayout = new QHBoxLayout;
    newHolderLayout->addWidget(previousHosts);
    newHolderLayout->addWidget(btnRefreshServers);

    auto *connectionLayout = new QGridLayout;
    connectionLayout->addLayout(newHolderLayout, 1, 1, 1, 2);
    connectionLayout->addWidget(saveLabel, 3, 0);
    connectionLayout->addWidget(saveEdit, 3, 1);
    connectionLayout->addWidget(hostLabel, 4, 0);
    connectionLayout->addWidget(hostEdit, 4, 1);
    connectionLayout->addWidget(portLabel, 5, 0);
    connectionLayout->addWidget(portEdit, 5, 1);
    connectionLayout->addWidget(autoConnectCheckBox, 6, 1);
    connectionLayout->addWidget(btnForgotPassword, 7, 1);

    auto *serversGroubBox = new QGroupBox(this);
    serversGroubBox->setTitle(tr("Server"));
    serversGroubBox->setLayout(connectionLayout);

    auto *serverInfoLayout = new QGridLayout;
    serverInfoLayout->addWidget(serverIssuesLabel, 0, 0, 1, 4, Qt::AlignTop);
    serverInfoLayout->addWidget(serverContactLabel, 1, 0);
    serverInfoLayout->addWidget(serverContactLink, 1, 1, 1, 3);

    auto *loginLayout = new QGridLayout;
    loginLayout->addWidget(playernameLabel, 0, 0);
    loginLayout->addWidget(playernameEdit, 0, 1);
    loginLayout->addWidget(passwordLabel, 1, 0);
    loginLayout->addWidget(passwordEdit, 1, 1);
    loginLayout->addWidget(passwordConfirmationLabel, 2, 0);
    loginLayout->addWidget(passwordConfirmationEdit, 2, 1);
    loginLayout->addWidget(countryLabel, 3, 0);
    loginLayout->addWidget(countryEdit, 3, 1);
    loginLayout->addWidget(emailLabel, 4, 0);
    loginLayout->addWidget(emailEdit, 4, 1);
    loginLayout->addWidget(realNameLabel, 5, 0);
    loginLayout->addWidget(realNameEdit, 5, 1);
    loginLayout->addWidget(savePasswordCheckBox, 6, 1);

    auto *loginGroupBox = new QGroupBox(this);
    loginGroupBox->setTitle(tr("Credentials"));
    loginGroupBox->setLayout(loginLayout);

    auto *serverInfoGroupBox = new QGroupBox(this);
    serverInfoGroupBox->setTitle(tr("Server Contact"));
    serverInfoGroupBox->setLayout(serverInfoLayout);

    auto *grid = new QGridLayout;
    grid->addWidget(serversGroubBox, 0, 0);
    grid->addWidget(serverInfoGroupBox, 1, 0);
    grid->addWidget(loginGroupBox, 2, 0);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);

    setWindowTitle(tr("Connect to Server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgConnect::selectServer(int /*index*/)
{
    currentSaveName = previousHosts->currentText();
    updateDisplayInfo();
}

void DlgConnect::updateSavePasswordEnabled()
{
    autoConnectCheckBox->setEnabled(savePasswordEnabled);
    autoConnectCheckBox->setChecked(savePasswordEnabled && autoConnectEnabled);
}

void DlgConnect::downloadThePublicServers()
{
    btnRefreshServers->setDisabled(true);
    hps.downloadPublicServers();
}

void DlgConnect::rebuildComboBoxList(int /*failure*/)
{
    auto &servers = SettingsCache::instance().servers();
    previousHosts->clear();
    for (int index = servers.start(); index < servers.end(); ++index) {
        QString saveName = servers.getSaveName(index);
        if (saveName.isEmpty() || previousHosts->findText(saveName) != -1) {
            qWarning() << "Corrupted server setting found at index" << index << "with name" << saveName;
        } else {
            previousHosts->addItem(saveName);
        }
    }
    previousHosts->setCurrentIndex(servers.getIndex(currentSaveName) - servers.start());

    // Re-enable the refresh server button
    btnRefreshServers->setDisabled(false);
}

void DlgConnect::addServer()
{
    saveEdit->clear();
    hostEdit->clear();
    portEdit->clear();
    editServerEnabled = true;
    updateEditServer();
}

void DlgConnect::updateEditServer()
{
    previousHosts->setDisabled(editServerEnabled);
    btnRefreshServers->setDisabled(editServerEnabled);
    hostEdit->setReadOnly(!editServerEnabled);
    portEdit->setReadOnly(!editServerEnabled);
    saveEdit->setReadOnly(!editServerEnabled);
    if (editServerEnabled) {
        playernameEdit->clear();
        passwordEdit->clear();
        saveEdit->clear();
    }
}

void DlgConnect::updateDisplayInfo()
{
    ServersSettings &servers = SettingsCache::instance().servers();
    int currentIndex = servers.getIndex(currentSaveName);
    savePasswordEnabled = servers.getSavePassword(currentIndex);
    saveEdit->setText(currentSaveName);
    hostEdit->setText(servers.getHostName(currentIndex));
    portEdit->setText(QString::number(servers.getPort(currentIndex)));
    QString playerName = servers.getPlayerName(currentIndex);
    playernameEdit->setText(playerName);
    savePasswordCheckBox->setChecked(savePasswordEnabled);
    passwordEdit->clear();

    if (savePasswordEnabled) {
        passwordEdit->setPlaceholderText(tr("password saved"));
    } else {
        passwordEdit->setPlaceholderText(QString());
    }

    QString site = servers.getSite(currentIndex);
    if (!site.isEmpty()) {
        QString formattedLink = QString("<a href=\"%1\">%1</a>").arg(site);
        serverContactLink->setText(formattedLink);
    } else {
        serverContactLabel->hide();
        serverContactLink->hide();
    }

    updateSavePasswordEnabled();

    if (playerName.isEmpty()) {
        playernameEdit->setFocus();
    } else {
        passwordEdit->setFocus();
    }
}

void DlgConnect::passwordSaved(int state)
{
    if (state) {
        autoConnectCheckBox->setEnabled(true);
    } else {
        autoConnectCheckBox->setChecked(false);
        autoConnectCheckBox->setEnabled(false);
    }
}

void DlgConnect::actOk()
{
    if(mode == Login) {
        QString password;
        if(savePasswordEnabled){
            password = QString();
        } else {
            password = passwordEdit->text();
        }
        client->connectToServer(hostEdit->text().trimmed(), portEdit->text().trimmed().toUInt(),
                                playernameEdit->text().trimmed(), password);
        //client->registerToServer(getHost(), static_cast<unsigned int>(dlg.getPort()), dlg.getPlayerName(), dlg.getPassword(), dlg.getEmail(), dlg.getCountry(), dlg.getRealName());
    }
}

void DlgConnect::connected()
{
    ServersSettings &servers = SettingsCache::instance().servers();
    QString serverName = saveEdit->text().trimmed();
    int index = servers.getIndex(serverName);
    if (index != -1) {
        servers.setHostName(index, hostEdit->text().trimmed());
        servers.setPort(index, portEdit->text().trimmed().toUInt());
        servers.setPlayerName(index, playernameEdit->text().trimmed());
        servers.setPreviousHostName(serverName);
        servers.setAutoConnect(autoConnectCheckBox->isChecked());
    }

    accept();
}

void DlgConnect::actForgotPassword()
{}
#if 0 // TODO
{
    ServersSettings &servers = SettingsCache::instance().servers();
    servers.setFPHostName(hostEdit->text());
    servers.setFPPort(portEdit->text());
    servers.setFPPlayerName(playernameEdit->text().trimmed());

    emit sigStartForgotPasswordRequest();
    reject();
}

void DlgConnect::actForgotPasswordRequest()
{
    DlgForgotPasswordRequest dlg(this);
    if (dlg.exec())
        client->requestForgotPasswordToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                              dlg.getPlayerName());
}

void DlgConnect::forgotPasswordSuccess()
{
    QMessageBox::information(
        this, tr("Reset Password"),
        tr("Your password has been reset successfully, you can now log in using the new credentials."));
    SettingsCache::instance().servers().setFPHostName("");
    SettingsCache::instance().servers().setFPPort("");
    SettingsCache::instance().servers().setFPPlayerName("");
}

void DlgConnect::forgotPasswordError()
{
    QMessageBox::warning(
        this, tr("Reset Password"),
        tr("Failed to reset user account password, please contact the server operator to reset your password."));
    SettingsCache::instance().servers().setFPHostName("");
    SettingsCache::instance().servers().setFPPort("");
    SettingsCache::instance().servers().setFPPlayerName("");
}

void DlgConnect::promptForgotPasswordReset()
{
    QMessageBox::information(this, tr("Reset Password"),
                             tr("Activation request received, please check your email for an activation token."));
    DlgForgotPasswordReset dlg(this);
    if (dlg.exec()) {
        client->submitForgotPasswordResetToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                  dlg.getPlayerName(), dlg.getToken(), dlg.getPassword());
    }
}
    connect(client, SIGNAL(sigForgotPasswordSuccess()), this, SLOT(forgotPasswordSuccess()));
    connect(client, SIGNAL(sigForgotPasswordError()), this, SLOT(forgotPasswordError()));
    connect(client, SIGNAL(sigPromptForForgotPasswordReset()), this, SLOT(promptForgotPasswordReset()));
    connect(client, SIGNAL(sigPromptForForgotPasswordChallenge()), this, SLOT(promptForgotPasswordChallenge()));
void MainWindow::promptForgotPasswordChallenge()
{
    DlgForgotPasswordChallenge dlg(this);
    if (dlg.exec())
        client->submitForgotPasswordChallengeToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                      dlg.getPlayerName(), dlg.getEmail());
}

    connect(client, SIGNAL(registerAccepted()), this, SLOT(registerAccepted()));
    connect(client, SIGNAL(registerAcceptedNeedsActivate()), this, SLOT(registerAcceptedNeedsActivate()));
    connect(client, SIGNAL(registerError(Response::ResponseCode, QString, quint32)), this,
            SLOT(registerError(Response::ResponseCode, QString, quint32)));

void MainWindow::registerAccepted()
{
    QMessageBox::information(this, tr("Success"), tr("Registration accepted.\nWill now login."));
}

void MainWindow::registerAcceptedNeedsActivate()
{
    // nothing
}

void MainWindow::registerError(Response::ResponseCode r, QString reasonStr, quint32 endTime)
{
    switch (r) {
        case Response::RespRegistrationDisabled:
            QMessageBox::critical(this, tr("Registration denied"),
                                  tr("Registration is currently disabled on this server"));
            break;
        case Response::RespUserAlreadyExists:
            QMessageBox::critical(this, tr("Registration denied"),
                                  tr("There is already an existing account with the same user name."));
            break;
        case Response::RespEmailRequiredToRegister:
            QMessageBox::critical(this, tr("Registration denied"),
                                  tr("It's mandatory to specify a valid email address when registering."));
            break;
        case Response::RespEmailBlackListed:
            if (reasonStr.isEmpty()) {
                reasonStr =
                    "The email address provider used during registration has been blocked from use on this server.";
            }
            QMessageBox::critical(this, tr("Registration denied"), reasonStr);
            break;
        case Response::RespTooManyRequests:
            QMessageBox::critical(
                this, tr("Registration denied"),
                tr("It appears you are attempting to register a new account on this server yet you already have an "
                   "account registered with the email provided. This server restricts the number of accounts a user "
                   "can register per address.  Please contact the server operator for further assistance or to obtain "
                   "your credential information."));
            break;
        case Response::RespPasswordTooShort:
            QMessageBox::critical(this, tr("Registration denied"), tr("Password too short."));
            break;
        case Response::RespUserIsBanned: {
            QString bannedStr;
            if (endTime)
                bannedStr = tr("You are banned until %1.").arg(QDateTime::fromTime_t(endTime).toString());
            else
                bannedStr = tr("You are banned indefinitely.");
            if (!reasonStr.isEmpty())
                bannedStr.append("\n\n" + reasonStr);

            QMessageBox::critical(this, tr("Error"), bannedStr);
            break;
        }
        case Response::RespUsernameInvalid: {
            QMessageBox::critical(this, tr("Error"), extractInvalidUsernameMessage(reasonStr));
            break;
        }
        case Response::RespRegistrationFailed:
            QMessageBox::critical(this, tr("Error"), tr("Registration failed for a technical problem on the server."));
            break;
        default:
            QMessageBox::critical(this, tr("Error"),
                                  tr("Unknown registration error: %1").arg(static_cast<int>(r)) +
                                      tr("\nThis usually means that your client version is out of date, and the server "
                                         "sent a reply your client doesn't understand."));
    }
    actRegister();
}

QString MainWindow::extractInvalidUsernameMessage(QString &in)
{
    QString out = tr("Invalid username.") + "<br/>";
    QStringList rules = in.split(QChar('|'));
    if (rules.size() == 7 || rules.size() == 9) {
        out += tr("Your username must respect these rules:") + "<ul>";

        out += "<li>" + tr("is %1 - %2 characters long").arg(rules.at(0)).arg(rules.at(1)) + "</li>";
        out += "<li>" + tr("can %1 contain lowercase characters").arg((rules.at(2).toInt() > 0) ? "" : tr("NOT")) +
               "</li>";
        out += "<li>" + tr("can %1 contain uppercase characters").arg((rules.at(3).toInt() > 0) ? "" : tr("NOT")) +
               "</li>";
        out +=
            "<li>" + tr("can %1 contain numeric characters").arg((rules.at(4).toInt() > 0) ? "" : tr("NOT")) + "</li>";

        if (rules.at(6).size() > 0)
            out += "<li>" + tr("can contain the following punctuation: %1").arg(rules.at(6).toHtmlEscaped()) + "</li>";

        out += "<li>" +
               tr("first character can %1 be a punctuation mark").arg((rules.at(5).toInt() > 0) ? "" : tr("NOT")) +
               "</li>";

        if (rules.size() == 9) {
            if (rules.at(7).size() > 0)
                out += "<li>" + tr("can not contain any of the following words: %1").arg(rules.at(7).toHtmlEscaped()) +
                       "</li>";

            if (rules.at(8).size() > 0)
                out += "<li>" +
                       tr("can not match any of the following expressions: %1").arg(rules.at(8).toHtmlEscaped()) +
                       "</li>";
        }

        out += "</ul>";
    } else {
        out += tr("You may only use A-Z, a-z, 0-9, _, ., and - in your username.");
    }

    return out;
}

void MainWindow::activateError()
{
    QMessageBox::critical(this, tr("Error"), tr("Account activation failed"));
    client->disconnectFromServer();
    actConnect();
}

void MainWindow::protocolVersionMismatch(int localVersion, int remoteVersion)
{
    if (localVersion > remoteVersion)
        QMessageBox::critical(this, tr("Error"),
                              tr("You are trying to connect to an obsolete server. Please downgrade your Cockatrice "
                                 "version or connect to a suitable server.\nLocal version is %1, remote version is %2.")
                                  .arg(localVersion)
                                  .arg(remoteVersion));
    else
        QMessageBox::critical(this, tr("Error"),
                              tr("Your Cockatrice client is obsolete. Please update your Cockatrice version.\nLocal "
                                 "version is %1, remote version is %2.")
                                  .arg(localVersion)
                                  .arg(remoteVersion));
}


void MainWindow::loginError(Response::ResponseCode r,
                            QString reasonStr,
                            quint32 endTime,
                            QList<QString> missingFeatures)
{
    switch (r) {
        case Response::RespClientUpdateRequired: {
            QString formattedMissingFeatures;
            formattedMissingFeatures = "Missing Features: ";
            for (int i = 0; i < missingFeatures.size(); ++i)
                formattedMissingFeatures.append(QString("\n     %1").arg(QChar(0x2022)) + " " +
                                                missingFeatures.value(i));

            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle(tr("Failed Login"));
            msgBox.setText(tr("Your client seems to be missing features this server requires for connection.") +
                           "\n\n" + tr("To update your client, go to 'Help -> Check for Client Updates'."));
            msgBox.setDetailedText(formattedMissingFeatures);
            msgBox.exec();
            break;
        }
        case Response::RespWrongPassword:
            QMessageBox::critical(
                this, tr("Error"),
                tr("Incorrect username or password. Please check your authentication information and try again."));
            break;
        case Response::RespWouldOverwriteOldSession:
            QMessageBox::critical(this, tr("Error"),
                                  tr("There is already an active session using this user name.\nPlease close that "
                                     "session first and re-login."));
            break;
        case Response::RespUserIsBanned: {
            QString bannedStr;
            if (endTime)
                bannedStr = tr("You are banned until %1.").arg(QDateTime::fromSecsSinceEpoch(endTime).toString());
            else
                bannedStr = tr("You are banned indefinitely.");
            if (!reasonStr.isEmpty())
                bannedStr.append("\n\n" + reasonStr);

            QMessageBox::critical(this, tr("Error"), bannedStr);
            break;
        }
        case Response::RespUsernameInvalid: {
            QMessageBox::critical(this, tr("Error"), extractInvalidUsernameMessage(reasonStr));
            break;
        }
        case Response::RespRegistrationRequired:
            if (QMessageBox::question(this, tr("Error"),
                                      tr("This server requires user registration. Do you want to register now?"),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                // actRegister(); //TODO
            }
            break;
        case Response::RespClientIdRequired:
            QMessageBox::critical(
                this, tr("Error"),
                tr("This server requires client IDs. Your client is either failing to generate an ID or you are "
                   "running a modified client.\nPlease close and reopen your client to try again."));
            break;
        case Response::RespContextError:
            QMessageBox::critical(this, tr("Error"),
                                  tr("An internal error has occurred, please close and reopen Cockatrice before trying "
                                     "again.\nIf the error persists, ensure you are running the latest version of the "
                                     "software and if needed contact the software developers."));
            break;
        case Response::RespAccountNotActivated: {
            bool ok = false;
            QString token = getTextWithMax(this, tr("Account activation"),
                                           tr("Your account has not been activated yet.\nYou need to provide "
                                              "the activation token received in the activation email."),
                                           QLineEdit::Normal, QString(), &ok);
            if (ok && !token.isEmpty()) {
                client->activateToServer(token);
                return;
            }
            client->disconnectFromServer();
            break;
        }
        case Response::RespServerFull: {
            QMessageBox::critical(this, tr("Server Full"),
                                  tr("The server has reached its maximum user capacity, please check back later."));
            break;
        }
        default:
            QMessageBox::critical(this, tr("Error"),
                                  tr("Unknown login error: %1").arg(static_cast<int>(r)) +
                                      tr("\nThis usually means that your client version is out of date, and the server "
                                         "sent a reply your client doesn't understand."));
            break;
    }
    actConnect();
}

void MainWindow::activateAccepted()
{
    QMessageBox::information(this, tr("Success"), tr("Account activation accepted.\nWill now login."));
}

#endif
