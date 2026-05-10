#include "remote_connection_controller.h"

#include "../../settings/cache_settings.h"
#include "../interface/widgets/dialogs/dlg_connect.h"
#include "../interface/widgets/dialogs/dlg_forgot_password_challenge.h"
#include "../interface/widgets/dialogs/dlg_forgot_password_request.h"
#include "../interface/widgets/dialogs/dlg_forgot_password_reset.h"
#include "../interface/widgets/dialogs/dlg_register.h"
#include "../interface/widgets/utility/get_text_with_max.h"

#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QThread>
#include <libcockatrice/network/client/remote/remote_client.h>
#include <libcockatrice/protocol/pb/response.pb.h>

ConnectionController::ConnectionController(QWidget *dialogParent, QObject *parent)
    : QObject(parent), dialogParent(dialogParent)
{
    remoteClient = new RemoteClient(nullptr, &SettingsCache::instance());

    clientThread = new QThread(this);
    remoteClient->moveToThread(clientThread);
    clientThread->start();

    wireClientSignals();
}

ConnectionController::~ConnectionController()
{
    remoteClient->deleteLater();
    clientThread->wait();
}

void ConnectionController::wireClientSignals()
{
    connect(remoteClient, &RemoteClient::connectionClosedEventReceived, this,
            &ConnectionController::onConnectionClosedEvent);

    connect(remoteClient, &RemoteClient::serverShutdownEventReceived, this,
            &ConnectionController::onServerShutdownEvent);

    connect(remoteClient, &RemoteClient::statusChanged, this, &ConnectionController::onStatusChanged);

    connect(remoteClient, &RemoteClient::userInfoChanged, this, &ConnectionController::onUserInfoReceived,
            Qt::BlockingQueuedConnection);

    connect(remoteClient, &RemoteClient::loginError, this,
            [this](Response::ResponseCode r, QString rs, quint32 et, QList<QString> mf) {
                onLoginError(static_cast<int>(r), rs, et, mf);
            });

    connect(remoteClient, &RemoteClient::registerError, this,
            [this](Response::ResponseCode r, QString rs, quint32 et) { onRegisterError(static_cast<int>(r), rs, et); });

    connect(remoteClient, &RemoteClient::activateError, this, &ConnectionController::onActivateError);
    connect(remoteClient, &RemoteClient::socketError, this, &ConnectionController::onSocketError);
    connect(remoteClient, &RemoteClient::serverTimeout, this, &ConnectionController::onServerTimeout);

    connect(remoteClient, &RemoteClient::protocolVersionMismatch, this,
            &ConnectionController::onProtocolVersionMismatch);

    connect(remoteClient, &RemoteClient::registerAccepted, this, &ConnectionController::onRegisterAccepted);

    connect(remoteClient, &RemoteClient::registerAcceptedNeedsActivate, this,
            &ConnectionController::onRegisterAcceptedNeedsActivate);

    connect(remoteClient, &RemoteClient::activateAccepted, this, &ConnectionController::onActivateAccepted);

    connect(remoteClient, &RemoteClient::notifyUserAboutUpdate, this, &ConnectionController::onNotifyUserAboutUpdate);

    connect(remoteClient, &RemoteClient::sigForgotPasswordSuccess, this,
            &ConnectionController::onForgotPasswordSuccess);

    connect(remoteClient, &RemoteClient::sigForgotPasswordError, this, &ConnectionController::onForgotPasswordError);

    connect(remoteClient, &RemoteClient::sigPromptForForgotPasswordReset, this,
            &ConnectionController::onPromptForgotPasswordReset);

    connect(remoteClient, &RemoteClient::sigPromptForForgotPasswordChallenge, this,
            &ConnectionController::onPromptForgotPasswordChallenge);
}

void ConnectionController::connectToServer()
{
    dlgConnect = new DlgConnect(dialogParent);
    connect(dlgConnect, &DlgConnect::sigStartForgotPasswordRequest, this, &ConnectionController::forgotPasswordRequest);

    if (dlgConnect->exec()) {
        remoteClient->connectToServer(dlgConnect->getHost(), static_cast<unsigned int>(dlgConnect->getPort()),
                                      dlgConnect->getPlayerName(), dlgConnect->getPassword());
    }
}

void ConnectionController::connectToServerDirect(const QString &host,
                                                 unsigned int port,
                                                 const QString &playerName,
                                                 const QString &password)
{
    remoteClient->connectToServer(host, port, playerName, password);
}

void ConnectionController::disconnectFromServer()
{
    remoteClient->disconnectFromServer();
}

void ConnectionController::registerToServer()
{
    DlgRegister dlg(dialogParent);
    if (dlg.exec()) {
        remoteClient->registerToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()), dlg.getPlayerName(),
                                       dlg.getPassword(), dlg.getEmail(), dlg.getCountry(), dlg.getRealName());
    }
}

void ConnectionController::forgotPasswordRequest()
{
    DlgForgotPasswordRequest dlg(dialogParent);
    if (dlg.exec()) {
        remoteClient->requestForgotPasswordToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                    dlg.getPlayerName());
    }
}

void ConnectionController::onConnectionClosedEvent(const Event_ConnectionClosed &event)
{
    remoteClient->disconnectFromServer();

    QString reasonStr;
    switch (event.reason()) {
        case Event_ConnectionClosed::USER_LIMIT_REACHED: {
            reasonStr = tr("The server has reached its maximum user capacity, please check back later.");
            break;
        }
        case Event_ConnectionClosed::TOO_MANY_CONNECTIONS: {
            reasonStr = tr("There are too many concurrent connections from your address.");
            break;
        }
        case Event_ConnectionClosed::BANNED: {
            reasonStr = tr("Banned by moderator");
            if (event.has_end_time())
                reasonStr.append(
                    "\n" + tr("Expected end time: %1").arg(QDateTime::fromSecsSinceEpoch(event.end_time()).toString()));
            else
                reasonStr.append("\n" + tr("This ban lasts indefinitely."));
            if (event.has_reason_str())
                reasonStr.append("\n\n" + QString::fromStdString(event.reason_str()));
            break;
        }
        case Event_ConnectionClosed::SERVER_SHUTDOWN: {
            reasonStr = tr("Scheduled server shutdown.");
            break;
        }
        case Event_ConnectionClosed::USERNAMEINVALID: {
            reasonStr = tr("Invalid username.");
            break;
        }
        case Event_ConnectionClosed::LOGGEDINELSEWERE: {
            reasonStr = tr("You have been logged out due to logging in at another location.");
            break;
        }
        default:
            reasonStr = QString::fromStdString(event.reason_str());
    }

    QMessageBox::critical(dialogParent, tr("Connection closed"),
                          tr("The server has terminated your connection.\nReason: %1").arg(reasonStr));
}

void ConnectionController::onServerShutdownEvent(const Event_ServerShutdown &event)
{
    serverShutdownMessageBox.setInformativeText(tr("The server is going to be restarted in %n minute(s).\nAll running "
                                                   "games will be lost.\nReason for shutdown: %1",
                                                   "", event.minutes())
                                                    .arg(QString::fromStdString(event.reason())));
    serverShutdownMessageBox.setIconPixmap(QPixmap("theme:cockatrice").scaled(64, 64));
    serverShutdownMessageBox.setText(tr("Scheduled server shutdown"));
    serverShutdownMessageBox.setWindowModality(Qt::ApplicationModal);
    serverShutdownMessageBox.setVisible(true);
}

void ConnectionController::onStatusChanged(ClientStatus status)
{
    // Update the window title first, then let MainWindow handle its own UI
    // state via the forwarded signal
    updateWindowTitle();
    emit statusChanged(status);

    // TabSupervisor::stop() needs calling on disconnect; start() is driven by
    // onUserInfoReceived → tabSupervisorStartRequested.
    if (status == StatusDisconnected) {
        emit tabSupervisorStopRequested();
    }
}

void ConnectionController::onUserInfoReceived(const ServerInfo_User &info)
{
    emit tabSupervisorStartRequested(info);
}

void ConnectionController::onLoginError(int r,
                                        QString reasonStr,
                                        quint32 endTime,
                                        const QList<QString> &missingFeatures)
{
    switch (static_cast<Response::ResponseCode>(r)) {
        case Response::RespClientUpdateRequired: {
            QString formatted = "Missing Features: ";
            for (int i = 0; i < missingFeatures.size(); ++i) {
                formatted.append(QString("\n     %1").arg(QChar(0x2022)) + " " + missingFeatures.value(i));
            }

            QMessageBox msgBox(dialogParent);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle(tr("Failed Login"));
            msgBox.setText(tr("Your client seems to be missing features this server requires for connection.") +
                           "\n\n" + tr("To update your client, go to 'Help -> Check for Client Updates'."));
            msgBox.setDetailedText(formatted);
            msgBox.exec();
            break;
        }

        case Response::RespWrongPassword: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("Incorrect username or password. "
                                     "Please check your authentication information and try again."));
            break;
        }

        case Response::RespWouldOverwriteOldSession: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("There is already an active session using this user name.\n"
                                     "Please close that session first and re-login."));
            break;
        }

        case Response::RespUserIsBanned: {
            QString bannedStr =
                endTime ? tr("You are banned until %1.").arg(QDateTime::fromSecsSinceEpoch(endTime).toString())
                        : tr("You are banned indefinitely.");
            if (!reasonStr.isEmpty())
                bannedStr.append("\n\n" + reasonStr);
            QMessageBox::critical(dialogParent, tr("Error"), bannedStr);
            break;
        }

        case Response::RespUsernameInvalid: {
            QMessageBox::critical(dialogParent, tr("Error"), extractInvalidUsernameMessage(reasonStr));
            break;
        }

        case Response::RespRegistrationRequired: {
            if (QMessageBox::question(dialogParent, tr("Error"),
                                      tr("This server requires user registration. Do you want to register now?"),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                registerToServer();
            }
            return; // don't re-prompt connect
        }

        case Response::RespClientIdRequired: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("This server requires client IDs. Your client is either failing to generate an "
                                     "ID or you are running a modified client.\n"
                                     "Please close and reopen your client to try again."));
            break;
        }

        case Response::RespContextError: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("An internal error has occurred, please close and reopen Cockatrice before "
                                     "trying again.\nIf the error persists, ensure you are running the latest "
                                     "version of the software and if needed contact the software developers."));
            break;
        }

        case Response::RespAccountNotActivated: {
            bool ok = false;
            QString token =
                getTextWithMax(dialogParent, tr("Account activation"),
                               tr("Your account has not been activated yet.\n"
                                  "You need to provide the activation token received in the activation email."),
                               QLineEdit::Normal, QString(), &ok);

            if (ok && !token.isEmpty()) {
                remoteClient->activateToServer(token);
                return;
            }
            remoteClient->disconnectFromServer();
            return;
        }

        case Response::RespServerFull: {
            QMessageBox::critical(dialogParent, tr("Server Full"),
                                  tr("The server has reached its maximum user capacity, please check back later."));
            break;
        }

        default: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("Unknown login error: %1").arg(r) +
                                      tr("\nThis usually means that your client version is out of date, and the server "
                                         "sent a reply your client doesn't understand."));
            break;
        }
    }

    // Re-open the connect dialog after any handled error
    connectToServer();
}

void ConnectionController::onRegisterError(int r, QString reasonStr, quint32 endTime)
{
    switch (static_cast<Response::ResponseCode>(r)) {
        case Response::RespRegistrationDisabled: {
            QMessageBox::critical(dialogParent, tr("Registration denied"),
                                  tr("Registration is currently disabled on this server"));
            break;
        }
        case Response::RespUserAlreadyExists: {
            QMessageBox::critical(dialogParent, tr("Registration denied"),
                                  tr("There is already an existing account with the same user name."));
            break;
        }
        case Response::RespEmailRequiredToRegister: {
            QMessageBox::critical(dialogParent, tr("Registration denied"),
                                  tr("It's mandatory to specify a valid email address when registering."));
            break;
        }
        case Response::RespEmailBlackListed: {
            if (reasonStr.isEmpty()) {
                reasonStr =
                    "The email address provider used during registration has been blocked from use on this server.";
            }
            QMessageBox::critical(dialogParent, tr("Registration denied"), reasonStr);
            break;
        }
        case Response::RespTooManyRequests: {
            QMessageBox::critical(dialogParent, tr("Registration denied"),
                                  tr("It appears you are attempting to register a new account on this server yet you "
                                     "already have an account registered with the email provided. This server "
                                     "restricts the number of accounts a user can register per address. Please "
                                     "contact the server operator for further assistance or to obtain your "
                                     "credential information."));
            break;
        }
        case Response::RespPasswordTooShort: {
            QMessageBox::critical(dialogParent, tr("Registration denied"), tr("Password too short."));
            break;
        }
        case Response::RespUserIsBanned: {
            QString bannedStr =
                endTime ? tr("You are banned until %1.").arg(QDateTime::fromSecsSinceEpoch(endTime).toString())
                        : tr("You are banned indefinitely.");
            if (!reasonStr.isEmpty())
                bannedStr.append("\n\n" + reasonStr);
            QMessageBox::critical(dialogParent, tr("Error"), bannedStr);
            break;
        }
        case Response::RespUsernameInvalid: {
            QMessageBox::critical(dialogParent, tr("Error"), extractInvalidUsernameMessage(reasonStr));
            break;
        }
        case Response::RespRegistrationFailed: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("Registration failed for a technical problem on the server."));
            break;
        }
        case Response::RespNotConnected: {
            QMessageBox::critical(dialogParent, tr("Error"), tr("The connection to the server has been lost."));
            break;
        }
        default: {
            QMessageBox::critical(dialogParent, tr("Error"),
                                  tr("Unknown registration error: %1").arg(r) +
                                      tr("\nThis usually means that your client version is out of date, and the server "
                                         "sent a reply your client doesn't understand."));
            break;
        }
    }

    registerToServer();
}

void ConnectionController::onActivateError()
{
    QMessageBox::critical(dialogParent, tr("Error"), tr("Account activation failed"));
    remoteClient->disconnectFromServer();
    connectToServer();
}

void ConnectionController::onSocketError(const QString &errorStr)
{
    QMessageBox::critical(dialogParent, tr("Error"), tr("Socket error: %1").arg(errorStr));
    connectToServer();
}

void ConnectionController::onServerTimeout()
{
    QMessageBox::critical(dialogParent, tr("Error"), tr("Server timeout"));
    connectToServer();
}

void ConnectionController::onProtocolVersionMismatch(int localVersion, int remoteVersion)
{
    if (localVersion > remoteVersion) {
        QMessageBox::critical(dialogParent, tr("Error"),
                              tr("You are trying to connect to an obsolete server. Please downgrade your Cockatrice "
                                 "version or connect to a suitable server.\n"
                                 "Local version is %1, remote version is %2.")
                                  .arg(localVersion)
                                  .arg(remoteVersion));
    } else {
        QMessageBox::critical(dialogParent, tr("Error"),
                              tr("Your Cockatrice client is obsolete. Please update your Cockatrice version.\n"
                                 "Local version is %1, remote version is %2.")
                                  .arg(localVersion)
                                  .arg(remoteVersion));
    }
}

void ConnectionController::onRegisterAccepted()
{
    QMessageBox::information(dialogParent, tr("Success"), tr("Registration accepted.\nWill now login."));
}

void ConnectionController::onRegisterAcceptedNeedsActivate()
{
    // Server will send activation email; nothing to display here.
}

void ConnectionController::onActivateAccepted()
{
    QMessageBox::information(dialogParent, tr("Success"), tr("Account activation accepted.\nWill now login."));
}

void ConnectionController::onNotifyUserAboutUpdate()
{
    QMessageBox::information(
        dialogParent, tr("Information"),
        tr("This server supports additional features that your client doesn't have.\n"
           "This is most likely not a problem, but this message might mean there is a new version of "
           "Cockatrice available or this server is running a custom or pre-release version.\n\n"
           "To update your client, go to Help -> Check for Updates."));
}

void ConnectionController::onForgotPasswordSuccess()
{
    QMessageBox::information(
        dialogParent, tr("Reset Password"),
        tr("Your password has been reset successfully, you can now log in using the new credentials."));
    SettingsCache::instance().servers().setFPHostName("");
    SettingsCache::instance().servers().setFPPort("");
    SettingsCache::instance().servers().setFPPlayerName("");
}

void ConnectionController::onForgotPasswordError()
{
    QMessageBox::warning(
        dialogParent, tr("Reset Password"),
        tr("Failed to reset user account password, please contact the server operator to reset your password."));
    SettingsCache::instance().servers().setFPHostName("");
    SettingsCache::instance().servers().setFPPort("");
    SettingsCache::instance().servers().setFPPlayerName("");
}

void ConnectionController::onPromptForgotPasswordReset()
{
    QMessageBox::information(dialogParent, tr("Reset Password"),
                             tr("Activation request received, please check your email for an activation token."));
    DlgForgotPasswordReset dlg(dialogParent);
    if (dlg.exec()) {
        remoteClient->submitForgotPasswordResetToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                        dlg.getPlayerName(), dlg.getToken(), dlg.getPassword());
    }
}

void ConnectionController::onPromptForgotPasswordChallenge()
{
    DlgForgotPasswordChallenge dlg(dialogParent);
    if (dlg.exec()) {
        remoteClient->submitForgotPasswordChallengeToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                            dlg.getPlayerName(), dlg.getEmail());
    }
}

void ConnectionController::updateWindowTitle()
{
    const QString appName = QStringLiteral("Cockatrice");
    QString title;

    switch (remoteClient->getStatus()) {
        case StatusConnecting: {
            title = appName + " - " + tr("Connecting to %1...").arg(remoteClient->peerName());
            break;
        }
        case StatusRegistering: {
            title = appName + " - " +
                    tr("Registering to %1 as %2...").arg(remoteClient->peerName()).arg(remoteClient->getUserName());
            break;
        }
        case StatusDisconnected: {
            title = appName + " - " + tr("Disconnected");
            break;
        }
        case StatusLoggingIn: {
            title = appName + " - " + tr("Connected, logging in at %1").arg(remoteClient->peerName());
            break;
        }
        case StatusLoggedIn: {
            title = remoteClient->getUserName() + "@" + remoteClient->peerName();
            break;
        }
        case StatusRequestingForgotPassword:
        case StatusSubmitForgotPasswordChallenge:
        case StatusSubmitForgotPasswordReset:
            title = appName + " - " +
                    tr("Requesting forgotten password to %1 as %2...")
                        .arg(remoteClient->peerName())
                        .arg(remoteClient->getUserName());
            break;
        default:
            title = appName;
    }

    emit windowTitleChanged(title);
}

// static
QString ConnectionController::extractInvalidUsernameMessage(QString &in)
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
            if (rules.at(7).size() > 0) {
                QString words = rules.at(7).toHtmlEscaped();
                if (words.startsWith("\n")) {
                    out += tr("no unacceptable language as specified by these server rules:",
                              "note that the following lines will not be translated");
                    for (QString &line : words.split("\n", Qt::SkipEmptyParts)) {
                        out += "<li>" + line + "</li>";
                    }
                } else {
                    out += "<li>" + tr("can not contain any of the following words: %1").arg(words) + "</li>";
                }
            }

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