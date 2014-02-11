#include <QTimer>
#include <QThread>
#include "remoteclient.h"

#include "pending_command.h"
#include "pb/commands.pb.h"
#include "pb/session_commands.pb.h"
#include "pb/response_login.pb.h"
#include "pb/server_message.pb.h"
#include "pb/event_server_identification.pb.h"

static const unsigned int protocolVersion = 14;

RemoteClient::RemoteClient(QObject *parent)
    : AbstractClient(parent), timeRunning(0), lastDataReceived(0), messageInProgress(false), handshakeStarted(false), messageLength(0)
{
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(ping()));

    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(socket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
    
    connect(this, SIGNAL(serverIdentificationEventReceived(const Event_ServerIdentification &)), this, SLOT(processServerIdentificationEvent(const Event_ServerIdentification &)));
    connect(this, SIGNAL(connectionClosedEventReceived(Event_ConnectionClosed)), this, SLOT(processConnectionClosedEvent(Event_ConnectionClosed)));
    connect(this, SIGNAL(sigConnectToServer(QString, unsigned int, QString, QString)), this, SLOT(doConnectToServer(QString, unsigned int, QString, QString)));
    connect(this, SIGNAL(sigDisconnectFromServer()), this, SLOT(doDisconnectFromServer()));
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

void RemoteClient::slotConnected()
{
    timeRunning = lastDataReceived = 0;
    timer->start();
    
    // dirty hack to be compatible with v14 server
    sendCommandContainer(CommandContainer());
    getNewCmdId();
    // end of hack
    
    setStatus(StatusAwaitingWelcome);
}

void RemoteClient::processServerIdentificationEvent(const Event_ServerIdentification &event)
{
    if (event.protocol_version() != protocolVersion) {
        emit protocolVersionMismatch(protocolVersion, event.protocol_version());
        setStatus(StatusDisconnecting);
        return;
    }
    setStatus(StatusLoggingIn);
    
    Command_Login cmdLogin;
    cmdLogin.set_user_name(userName.toStdString());
    cmdLogin.set_password(password.toStdString());
    
    PendingCommand *pend = prepareSessionCommand(cmdLogin);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(loginResponse(Response)));
    sendCommand(pend);
}

void RemoteClient::processConnectionClosedEvent(const Event_ConnectionClosed & /*event*/)
{
    doDisconnectFromServer();
}

void RemoteClient::loginResponse(const Response &response)
{
    const Response_Login &resp = response.GetExtension(Response_Login::ext);
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
    } else {
        emit loginError(response.response_code(), QString::fromStdString(resp.denied_reason_str()), resp.denied_end_time());
        setStatus(StatusDisconnecting);
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
                    messageLength =   (((quint32) (unsigned char) inputBuffer[0]) << 24)
                            + (((quint32) (unsigned char) inputBuffer[1]) << 16)
                            + (((quint32) (unsigned char) inputBuffer[2]) << 8)
                            + ((quint32) (unsigned char) inputBuffer[3]);
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
#ifdef QT_DEBUG
        qDebug() << "IN" << messageLength << QString::fromStdString(newServerMessage.ShortDebugString());
#endif
        inputBuffer.remove(0, messageLength);
        messageInProgress = false;
        
        processProtocolItem(newServerMessage);
    
        if (getStatus() == StatusDisconnecting) // use thread-safe getter
            doDisconnectFromServer();
    } while (!inputBuffer.isEmpty());
}

void RemoteClient::sendCommandContainer(const CommandContainer &cont)
{
    QByteArray buf;
    unsigned int size = cont.ByteSize();
#ifdef QT_DEBUG
    qDebug() << "OUT" << size << QString::fromStdString(cont.ShortDebugString());
#endif
    buf.resize(size + 4);
    cont.SerializeToArray(buf.data() + 4, size);
    buf.data()[3] = (unsigned char) size;
    buf.data()[2] = (unsigned char) (size >> 8);
    buf.data()[1] = (unsigned char) (size >> 16);
    buf.data()[0] = (unsigned char) (size >> 24);
    
    socket->write(buf);
}

void RemoteClient::doConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password)
{
    doDisconnectFromServer();
    
    userName = _userName;
    password = _password;
    socket->connectToHost(hostname, port);
    setStatus(StatusConnecting);
}

void RemoteClient::doDisconnectFromServer()
{
    timer->stop();
    
    messageInProgress = false;
    handshakeStarted = false;
    messageLength = 0;

    QList<PendingCommand *> pc = pendingCommands.values();
    for (int i = 0; i < pc.size(); i++) {
        Response response;
        response.set_response_code(Response::RespNotConnected);
        response.set_cmd_id(pc[i]->getCommandContainer().cmd_id());
        pc[i]->processResponse(response);
        
        delete pc[i];
    }
    pendingCommands.clear();

    setStatus(StatusDisconnected);
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

void RemoteClient::connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password)
{
    emit sigConnectToServer(hostname, port, _userName, _password);
}

void RemoteClient::disconnectFromServer()
{
    emit sigDisconnectFromServer();
}
