#include "remote_client.h"

#include "version_string.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QHostAddress>
#include <QHostInfo>
#include <QList>
#include <QThread>
#include <QTimer>
#include <QWebSocket>
#include <libcockatrice/interfaces/interface_network_settings_provider.h>
#include <libcockatrice/protocol/debug_pb_message.h>
#include <libcockatrice/protocol/pb/event_server_identification.pb.h>
#include <libcockatrice/protocol/pb/response_activate.pb.h>
#include <libcockatrice/protocol/pb/response_forgotpasswordrequest.pb.h>
#include <libcockatrice/protocol/pb/response_login.pb.h>
#include <libcockatrice/protocol/pb/response_password_salt.pb.h>
#include <libcockatrice/protocol/pb/response_register.pb.h>
#include <libcockatrice/protocol/pb/server_message.pb.h>
#include <libcockatrice/protocol/pb/session_commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <libcockatrice/utility/passwordhasher.h>

static const unsigned int protocolVersion = 14;

RemoteClient::RemoteClient(QObject *parent, INetworkSettingsProvider *_networkSettingsProvider)
    : AbstractClient(parent), networkSettingsProvider(_networkSettingsProvider), timeRunning(0), lastDataReceived(0),
      messageInProgress(false), handshakeStarted(false), usingWebSocket(false), messageLength(0), hashedPassword()
{

    clearNewClientFeatures();
    maxTimeout = networkSettingsProvider->getTimeOut();
    int keepalive = networkSettingsProvider->getKeepAlive();
    timer = new QTimer(this);
    timer->setInterval(keepalive * 1000);
    connect(timer, &QTimer::timeout, this, &RemoteClient::ping);

    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket, &QTcpSocket::connected, this, &RemoteClient::slotConnected);
    connect(socket, &QTcpSocket::readyRead, this, &RemoteClient::readData);

    connect(socket, &QTcpSocket::errorOccurred, this, &RemoteClient::slotSocketError);

    websocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(websocket, &QWebSocket::binaryMessageReceived, this, &RemoteClient::websocketMessageReceived);
    connect(websocket, &QWebSocket::connected, this, &RemoteClient::slotConnected);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    connect(websocket, &QWebSocket::errorOccurred, this, &RemoteClient::slotWebSocketError);
#else
    connect(websocket, qOverload<QAbstractSocket::SocketError>(&QWebSocket::error), this,
            &RemoteClient::slotWebSocketError);
#endif

    connect(this, &RemoteClient::serverIdentificationEventReceived, this,
            &RemoteClient::processServerIdentificationEvent);
    connect(this, &RemoteClient::connectionClosedEventReceived, this, &RemoteClient::processConnectionClosedEvent);
    connect(this, &RemoteClient::sigConnectToServer, this, &RemoteClient::doConnectToServer);
    connect(this, &RemoteClient::sigDisconnectFromServer, this, &RemoteClient::doDisconnectFromServer);
    connect(this, &RemoteClient::sigRegisterToServer, this, &RemoteClient::doRegisterToServer);
    connect(this, &RemoteClient::sigActivateToServer, this, &RemoteClient::doActivateToServer);
    connect(this, &RemoteClient::sigRequestForgotPasswordToServer, this,
            &RemoteClient::doRequestForgotPasswordToServer);
    connect(this, &RemoteClient::sigSubmitForgotPasswordResetToServer, this,
            &RemoteClient::doSubmitForgotPasswordResetToServer);
    connect(this, &RemoteClient::sigSubmitForgotPasswordChallengeToServer, this,
            &RemoteClient::doSubmitForgotPasswordChallengeToServer);
}

RemoteClient::~RemoteClient()
{
    doDisconnectFromServer();
    thread()->quit();
}

void RemoteClient::slotSocketError(QAbstractSocket::SocketError /*error*/)
{
    QString errorString = socket->errorString();
    doDisconnectFromServer();
    emit socketError(errorString);
}

void RemoteClient::slotWebSocketError(QAbstractSocket::SocketError /*error*/)
{

    QString errorString = websocket->errorString();
    if (getStatus() != ClientStatus::StatusDisconnected) {
        doDisconnectFromServer();
        emit socketError(errorString);
    }
}

void RemoteClient::slotConnected()
{
    timeRunning = lastDataReceived = 0;
    timer->start();

    if (!usingWebSocket) {
        // dirty hack to be compatible with v14 server
        sendCommandContainer(CommandContainer());
        getNewCmdId();
        // end of hack
    }
}

void RemoteClient::processServerIdentificationEvent(const Event_ServerIdentification &event)
{
    if (event.protocol_version() != protocolVersion) {
        emit protocolVersionMismatch(protocolVersion, event.protocol_version());
        setStatus(StatusDisconnecting);
        return;
    }
    serverSupportsPasswordHash = event.server_options() & Event_ServerIdentification::SupportsPasswordHash;

    if (getStatus() == StatusRequestingForgotPassword) {
        Command_ForgotPasswordRequest cmdForgotPasswordRequest;
        cmdForgotPasswordRequest.set_user_name(userName.toStdString());
        cmdForgotPasswordRequest.set_clientid(getSrvClientID(lastHostname).toStdString());
        PendingCommand *pend = prepareSessionCommand(cmdForgotPasswordRequest);
        connect(pend, &PendingCommand::finished, this, &RemoteClient::requestForgotPasswordResponse);
        sendCommand(pend);
        return;
    }

    if (getStatus() == StatusSubmitForgotPasswordReset) {
        Command_ForgotPasswordReset cmdForgotPasswordReset;
        cmdForgotPasswordReset.set_user_name(userName.toStdString());
        cmdForgotPasswordReset.set_clientid(getSrvClientID(lastHostname).toStdString());
        cmdForgotPasswordReset.set_token(token.toStdString());
        if (!password.isEmpty() && serverSupportsPasswordHash) {
            auto passwordSalt = PasswordHasher::generateRandomSalt();
            hashedPassword = PasswordHasher::computeHash(password, passwordSalt);
            cmdForgotPasswordReset.set_hashed_new_password(hashedPassword.toStdString());
        } else if (!password.isEmpty()) {
            qCWarning(RemoteClientLog) << "using plain text password to reset password";
            cmdForgotPasswordReset.set_new_password(password.toStdString());
        }
        PendingCommand *pend = prepareSessionCommand(cmdForgotPasswordReset);
        connect(pend, &PendingCommand::finished, this, &RemoteClient::submitForgotPasswordResetResponse);
        sendCommand(pend);
        return;
    }

    if (getStatus() == StatusSubmitForgotPasswordChallenge) {
        Command_ForgotPasswordChallenge cmdForgotPasswordChallenge;
        cmdForgotPasswordChallenge.set_user_name(userName.toStdString());
        cmdForgotPasswordChallenge.set_clientid(getSrvClientID(lastHostname).toStdString());
        cmdForgotPasswordChallenge.set_email(email.toStdString());
        PendingCommand *pend = prepareSessionCommand(cmdForgotPasswordChallenge);
        connect(pend, &PendingCommand::finished, this, &RemoteClient::submitForgotPasswordChallengeResponse);
        sendCommand(pend);
        return;
    }

    if (getStatus() == StatusRegistering) {
        Command_Register cmdRegister;
        cmdRegister.set_user_name(userName.toStdString());
        if (!password.isEmpty() && serverSupportsPasswordHash) {
            auto passwordSalt = PasswordHasher::generateRandomSalt();
            hashedPassword = PasswordHasher::computeHash(password, passwordSalt);
            cmdRegister.set_hashed_password(hashedPassword.toStdString());
        } else if (!password.isEmpty()) {
            qCWarning(RemoteClientLog) << "using plain text password to register";
            cmdRegister.set_password(password.toStdString());
        }
        cmdRegister.set_email(email.toStdString());
        cmdRegister.set_country(country.toStdString());
        cmdRegister.set_real_name(realName.toStdString());
        cmdRegister.set_clientid(getSrvClientID(lastHostname).toStdString());
        PendingCommand *pend = prepareSessionCommand(cmdRegister);
        connect(pend, &PendingCommand::finished, this, &RemoteClient::registerResponse);
        sendCommand(pend);

        return;
    }

    if (getStatus() == StatusActivating) {
        Command_Activate cmdActivate;
        cmdActivate.set_user_name(userName.toStdString());
        cmdActivate.set_token(token.toStdString());
        cmdActivate.set_clientid(getSrvClientID(lastHostname).toStdString());

        PendingCommand *pend = prepareSessionCommand(cmdActivate);
        connect(pend, &PendingCommand::finished, this, &RemoteClient::activateResponse);
        sendCommand(pend);

        return;
    }

    doLogin();
}

void RemoteClient::doRequestPasswordSalt()
{
    setStatus(StatusGettingPasswordSalt);
    Command_RequestPasswordSalt cmdRqSalt;
    cmdRqSalt.set_user_name(userName.toStdString());

    PendingCommand *pend = prepareSessionCommand(cmdRqSalt);
    connect(pend, &PendingCommand::finished, this, &RemoteClient::passwordSaltResponse);
    sendCommand(pend);
}

Command_Login RemoteClient::generateCommandLogin()
{
    Command_Login cmdLogin;
    cmdLogin.set_user_name(userName.toStdString());
    cmdLogin.set_clientid(getSrvClientID(lastHostname).toStdString());
    cmdLogin.set_clientver(VERSION_STRING);

    if (!clientFeatures.isEmpty()) {
        QMap<QString, bool>::iterator i;
        for (i = clientFeatures.begin(); i != clientFeatures.end(); ++i)
            cmdLogin.add_clientfeatures(i.key().toStdString().c_str());
    }

    return cmdLogin;
}

void RemoteClient::doLogin()
{
    if (!password.isEmpty() && serverSupportsPasswordHash) {
        // TODO store and log in using stored hashed password
        if (hashedPassword.isEmpty()) {
            doRequestPasswordSalt(); // ask salt to create hashedPassword, then log in
        } else {
            doHashedLogin(); // log in using hashed password instead
        }
    } else {
        // TODO add setting for client to reject unhashed logins
        setStatus(StatusLoggingIn);
        Command_Login cmdLogin = generateCommandLogin();
        if (!password.isEmpty()) {
            qCWarning(RemoteClientLog) << "using plain text password to log in";
            cmdLogin.set_password(password.toStdString());
        }

        PendingCommand *pend = prepareSessionCommand(cmdLogin);
        connect(pend, &PendingCommand::finished, this, &RemoteClient::loginResponse);
        sendCommand(pend);
    }
}

void RemoteClient::doHashedLogin()
{
    setStatus(StatusLoggingIn);
    Command_Login cmdLogin = generateCommandLogin();

    cmdLogin.set_hashed_password(hashedPassword.toStdString());

    PendingCommand *pend = prepareSessionCommand(cmdLogin);
    connect(pend, &PendingCommand::finished, this, &RemoteClient::loginResponse);
    sendCommand(pend);
}

void RemoteClient::processConnectionClosedEvent(const Event_ConnectionClosed & /*event*/)
{
    doDisconnectFromServer();
}

void RemoteClient::passwordSaltResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        const Response_PasswordSalt &resp = response.GetExtension(Response_PasswordSalt::ext);
        auto passwordSalt = QString::fromStdString(resp.password_salt());
        if (passwordSalt.isEmpty()) { // the server does not recognize the user but allows them to enter unregistered
            password.clear();         // the password will not be used
            doLogin();
        } else {
            hashedPassword = PasswordHasher::computeHash(password, passwordSalt);
            doHashedLogin();
        }
    } else if (response.response_code() != Response::RespNotConnected) {
        emit loginError(response.response_code(), {}, 0, {});
    }
}

void RemoteClient::loginResponse(const Response &response)
{
    const Response_Login &resp = response.GetExtension(Response_Login::ext);

    QString possibleMissingFeatures;
    if (resp.missing_features_size() > 0) {
        for (int i = 0; i < resp.missing_features_size(); ++i)
            possibleMissingFeatures.append("," + QString::fromStdString(resp.missing_features(i)));
    }

    if (response.response_code() == Response::RespOk) {
        setStatus(StatusLoggedIn);
        emit userInfoChanged(resp.user_info());

        QList<ServerInfo_User> buddyList;
        for (int i = resp.buddy_list_size() - 1; i >= 0; --i)
            buddyList.append(resp.buddy_list(i));
        emit buddyListReceived(buddyList);

        QList<ServerInfo_User> ignoreList;
        for (int i = resp.ignore_list_size() - 1; i >= 0; --i)
            ignoreList.append(resp.ignore_list(i));
        emit ignoreListReceived(ignoreList);

        if (newMissingFeatureFound(possibleMissingFeatures) && resp.missing_features_size() > 0 &&
            networkSettingsProvider->getNotifyAboutUpdates()) {
            networkSettingsProvider->setKnownMissingFeatures(possibleMissingFeatures);
            emit notifyUserAboutUpdate();
        }

    } else if (response.response_code() != Response::RespNotConnected) {
        QList<QString> missingFeatures;
        if (resp.missing_features_size() > 0) {
            for (int i = 0; i < resp.missing_features_size(); ++i)
                missingFeatures << QString::fromStdString(resp.missing_features(i));
        }
        emit loginError(response.response_code(), QString::fromStdString(resp.denied_reason_str()),
                        static_cast<quint32>(resp.denied_end_time()), missingFeatures);
        setStatus(StatusDisconnecting);
    }
}

void RemoteClient::registerResponse(const Response &response)
{
    const Response_Register &resp = response.GetExtension(Response_Register::ext);
    switch (response.response_code()) {
        case Response::RespRegistrationAccepted:
            emit registerAccepted();
            doLogin();
            break;
        case Response::RespRegistrationAcceptedNeedsActivation:
            emit registerAcceptedNeedsActivate();
            doLogin();
            break;
        case Response::RespNotConnected:
            // this response is created by the client from doDisconnectFromServer, do not call it again!
            emit registerError(response.response_code(), QString::fromStdString(resp.denied_reason_str()),
                               static_cast<quint32>(resp.denied_end_time()));
            break;
        default:
            emit registerError(response.response_code(), QString::fromStdString(resp.denied_reason_str()),
                               static_cast<quint32>(resp.denied_end_time()));
            setStatus(StatusDisconnecting);
            doDisconnectFromServer();
            break;
    }
}

void RemoteClient::activateResponse(const Response &response)
{
    if (response.response_code() == Response::RespActivationAccepted) {
        emit activateAccepted();

        doLogin();
    } else {
        emit activateError();
    }
}

void RemoteClient::readData()
{
    lastDataReceived = timeRunning;
    QByteArray data = socket->readAll();

    inputBuffer.append(data);

    do {
        if (!messageInProgress) {
            if (inputBuffer.size() >= 4) {
                // dirty hack to be compatible with v14 server that sends 60 bytes of garbage at the beginning
                if (!handshakeStarted) {
                    handshakeStarted = true;
                    if (inputBuffer.startsWith("<?xm")) {
                        messageInProgress = true;
                        messageLength = 60;
                    }
                } else {
                    // end of hack
                    messageLength = (((quint32)(unsigned char)inputBuffer[0]) << 24) +
                                    (((quint32)(unsigned char)inputBuffer[1]) << 16) +
                                    (((quint32)(unsigned char)inputBuffer[2]) << 8) +
                                    ((quint32)(unsigned char)inputBuffer[3]);
                    inputBuffer.remove(0, 4);
                    messageInProgress = true;
                }
            } else
                return;
        }
        if (inputBuffer.size() < messageLength)
            return;

        ServerMessage newServerMessage;
        newServerMessage.ParseFromArray(inputBuffer.data(), messageLength);

        qCDebug(RemoteClientLog).noquote() << "IN" << getSafeDebugString(newServerMessage);

        inputBuffer.remove(0, messageLength);
        messageInProgress = false;

        processProtocolItem(newServerMessage);

        if (getStatus() == StatusDisconnecting) // use thread-safe getter
            doDisconnectFromServer();
    } while (!inputBuffer.isEmpty());
}

void RemoteClient::websocketMessageReceived(const QByteArray &message)
{
    lastDataReceived = timeRunning;
    ServerMessage newServerMessage;
    newServerMessage.ParseFromArray(message.data(), message.length());

    qCDebug(RemoteClientLog).noquote() << "IN" << getSafeDebugString(newServerMessage);

    processProtocolItem(newServerMessage);
}

void RemoteClient::sendCommandContainer(const CommandContainer &cont)
{
#if GOOGLE_PROTOBUF_VERSION > 3001000
    auto size = static_cast<unsigned int>(cont.ByteSizeLong());
#else
    auto size = static_cast<unsigned int>(cont.ByteSize());
#endif

    qCDebug(RemoteClientLog).noquote() << "OUT" << getSafeDebugString(cont);

    QByteArray buf;
    if (usingWebSocket) {
        buf.resize(size);
        cont.SerializeToArray(buf.data(), size);
        websocket->sendBinaryMessage(buf);
    } else {
        buf.resize(size + 4);
        cont.SerializeToArray(buf.data() + 4, size);
        buf.data()[3] = (unsigned char)size;
        buf.data()[2] = (unsigned char)(size >> 8);
        buf.data()[1] = (unsigned char)(size >> 16);
        buf.data()[0] = (unsigned char)(size >> 24);

        socket->write(buf);
    }
}

void RemoteClient::connectToHost(const QString &hostname, unsigned int port)
{
    usingWebSocket = port == 443 || port == 80 || port == 4748 || port == 8080;
    if (usingWebSocket) {
        QUrl url(QString("%1://%2:%3/servatrice").arg(port == 443 ? "wss" : "ws").arg(hostname).arg(port));
        websocket->open(url);
    } else {
        socket->connectToHost(hostname, static_cast<quint16>(port));
    }
}

void RemoteClient::doConnectToServer(const QString &hostname,
                                     unsigned int port,
                                     const QString &_userName,
                                     const QString &_password)
{
    doDisconnectFromServer();

    userName = _userName;
    password = _password;
    lastHostname = hostname;
    lastPort = port;
    hashedPassword.clear();

    connectToHost(hostname, port);
    setStatus(StatusConnecting);
}

void RemoteClient::doRegisterToServer(const QString &hostname,
                                      unsigned int port,
                                      const QString &_userName,
                                      const QString &_password,
                                      const QString &_email,
                                      const QString &_country,
                                      const QString &_realname)
{
    doDisconnectFromServer();

    userName = _userName;
    password = _password;
    email = _email;
    country = _country;
    realName = _realname;
    lastHostname = hostname;
    lastPort = port;
    hashedPassword.clear();

    connectToHost(hostname, port);
    setStatus(StatusRegistering);
}

void RemoteClient::doActivateToServer(const QString &_token)
{
    doDisconnectFromServer();

    token = _token.trimmed();

    connectToHost(lastHostname, lastPort);
    setStatus(StatusActivating);
}

void RemoteClient::doDisconnectFromServer()
{
    timer->stop();

    messageInProgress = false;
    handshakeStarted = false;
    messageLength = 0;

    QList<PendingCommand *> pc = pendingCommands.values();
    for (const auto &i : pc) {
        Response response;
        response.set_response_code(Response::RespNotConnected);
        response.set_cmd_id(i->getCommandContainer().cmd_id());
        i->processResponse(response);

        delete i;
    }
    pendingCommands.clear();

    setStatus(StatusDisconnected);
    if (websocket->isValid())
        websocket->close();
    socket->close();
}

void RemoteClient::ping()
{
    QMutableMapIterator<int, PendingCommand *> i(pendingCommands);
    while (i.hasNext()) {
        PendingCommand *pend = i.next().value();
        if (pend->tick() > maxTimeout) {
            i.remove();
            pend->deleteLater();
        }
    }

    int maxTime = timeRunning - lastDataReceived;
    emit maxPingTime(maxTime, maxTimeout);
    if (maxTime >= maxTimeout) {
        disconnectFromServer();
        emit serverTimeout();
    } else {
        sendCommand(prepareSessionCommand(Command_Ping()));
        ++timeRunning;
    }
}

void RemoteClient::connectToServer(const QString &hostname,
                                   unsigned int port,
                                   const QString &_userName,
                                   const QString &_password)
{
    emit sigConnectToServer(hostname, port, _userName, _password);
}

void RemoteClient::registerToServer(const QString &hostname,
                                    unsigned int port,
                                    const QString &_userName,
                                    const QString &_password,
                                    const QString &_email,
                                    const QString &_country,
                                    const QString &_realname)
{
    emit sigRegisterToServer(hostname, port, _userName, _password, _email, _country, _realname);
}

void RemoteClient::activateToServer(const QString &_token)
{
    emit sigActivateToServer(_token.trimmed());
}

void RemoteClient::disconnectFromServer()
{
    emit sigDisconnectFromServer();
}

QString RemoteClient::getSrvClientID(const QString &_hostname)
{
    QString srvClientID = networkSettingsProvider->getClientID();
    QHostInfo hostInfo = QHostInfo::fromName(_hostname);
    if (!hostInfo.error()) {
        QHostAddress hostAddress = hostInfo.addresses().first();
        srvClientID += hostAddress.toString();
    } else {
        qCWarning(RemoteClientLog) << "ClientID generation host lookup failure [" << hostInfo.errorString() << "]";
        srvClientID += _hostname;
    }
    QString uniqueServerClientID =
        QCryptographicHash::hash(srvClientID.toUtf8(), QCryptographicHash::Sha1).toHex().right(15);
    return uniqueServerClientID;
}

bool RemoteClient::newMissingFeatureFound(const QString &_serversMissingFeatures)
{
    bool newMissingFeature = false;
    QStringList serversMissingFeaturesList = _serversMissingFeatures.split(",");
    for (const QString &feature : serversMissingFeaturesList) {
        if (!feature.isEmpty()) {
            if (!networkSettingsProvider->getKnownMissingFeatures().contains(feature))
                return true;
        }
    }
    return newMissingFeature;
}

void RemoteClient::clearNewClientFeatures()
{
    QString newKnownMissingFeatures;
    QStringList existingKnownMissingFeatures = networkSettingsProvider->getKnownMissingFeatures().split(",");
    for (const QString &existingKnownFeature : existingKnownMissingFeatures) {
        if (!existingKnownFeature.isEmpty()) {
            if (!clientFeatures.contains(existingKnownFeature))
                newKnownMissingFeatures.append("," + existingKnownFeature);
        }
    }
    networkSettingsProvider->setKnownMissingFeatures(newKnownMissingFeatures);
}

void RemoteClient::requestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName)
{
    emit sigRequestForgotPasswordToServer(hostname, port, _userName);
}

void RemoteClient::submitForgotPasswordResetToServer(const QString &hostname,
                                                     unsigned int port,
                                                     const QString &_userName,
                                                     const QString &_token,
                                                     const QString &_newpassword)
{
    emit sigSubmitForgotPasswordResetToServer(hostname, port, _userName, _token.trimmed(), _newpassword);
}

void RemoteClient::doRequestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName)
{
    doDisconnectFromServer();

    userName = _userName;
    lastHostname = hostname;
    lastPort = port;

    connectToHost(lastHostname, lastPort);
    setStatus(StatusRequestingForgotPassword);
}

void RemoteClient::requestForgotPasswordResponse(const Response &response)
{
    const Response_ForgotPasswordRequest &resp = response.GetExtension(Response_ForgotPasswordRequest::ext);
    if (response.response_code() == Response::RespOk) {
        if (resp.challenge_email()) {
            emit sigPromptForForgotPasswordChallenge();
        } else
            emit sigPromptForForgotPasswordReset();
    } else
        emit sigForgotPasswordError();

    doDisconnectFromServer();
}

void RemoteClient::doSubmitForgotPasswordResetToServer(const QString &hostname,
                                                       unsigned int port,
                                                       const QString &_userName,
                                                       const QString &_token,
                                                       const QString &_newpassword)
{
    doDisconnectFromServer();

    userName = _userName;
    lastHostname = hostname;
    lastPort = port;
    token = _token.trimmed();
    password = _newpassword;
    hashedPassword.clear();

    connectToHost(lastHostname, lastPort);
    setStatus(StatusSubmitForgotPasswordReset);
}

void RemoteClient::submitForgotPasswordResetResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        emit sigForgotPasswordSuccess();
    } else
        emit sigForgotPasswordError();

    doDisconnectFromServer();
}

void RemoteClient::submitForgotPasswordChallengeToServer(const QString &hostname,
                                                         unsigned int port,
                                                         const QString &_userName,
                                                         const QString &_email)
{
    emit sigSubmitForgotPasswordChallengeToServer(hostname, port, _userName, _email);
}

void RemoteClient::doSubmitForgotPasswordChallengeToServer(const QString &hostname,
                                                           unsigned int port,
                                                           const QString &_userName,
                                                           const QString &_email)
{
    doDisconnectFromServer();

    userName = _userName;
    lastHostname = hostname;
    lastPort = port;
    email = _email;

    connectToHost(lastHostname, lastPort);
    setStatus(StatusSubmitForgotPasswordChallenge);
}

void RemoteClient::submitForgotPasswordChallengeResponse(const Response &response)
{
    if (response.response_code() == Response::RespOk) {
        emit sigPromptForForgotPasswordReset();
    } else
        emit sigForgotPasswordError();

    doDisconnectFromServer();
}
