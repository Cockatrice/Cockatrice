/**
 * @file remote_client.h
 * @ingroup Client
 * @brief TODO: Document this.
 */

#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include "../abstract/abstract_client.h"

#include <QLoggingCategory>
#include <QTcpSocket>
#include <QWebSocket>
#include <libcockatrice/interfaces/interface_network_settings_provider.h>
#include <libcockatrice/protocol/pb/commands.pb.h>

inline Q_LOGGING_CATEGORY(RemoteClientLog, "remote_client");

class QTimer;

class RemoteClient : public AbstractClient
{
    Q_OBJECT
signals:
    void serverTimeout();
    void loginError(Response::ResponseCode resp, QString reasonStr, quint32 endTime, QList<QString> missingFeatures);
    void registerError(Response::ResponseCode resp, QString reasonStr, quint32 endTime);
    void activateError();
    void socketError(const QString &errorString);
    void protocolVersionMismatch(int clientVersion, int serverVersion);
    void
    sigConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void sigRegisterToServer(const QString &hostname,
                             unsigned int port,
                             const QString &_userName,
                             const QString &_password,
                             const QString &_email,
                             const QString &_country,
                             const QString &_realname);
    void sigActivateToServer(const QString &_token);
    void sigDisconnectFromServer();
    void notifyUserAboutUpdate();
    void sigRequestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName);
    void sigForgotPasswordSuccess();
    void sigForgotPasswordError();
    void sigPromptForForgotPasswordReset();
    void sigSubmitForgotPasswordResetToServer(const QString &hostname,
                                              unsigned int port,
                                              const QString &_userName,
                                              const QString &_token,
                                              const QString &_newpassword);
    void sigPromptForForgotPasswordChallenge();
    void sigSubmitForgotPasswordChallengeToServer(const QString &hostname,
                                                  unsigned int port,
                                                  const QString &_userName,
                                                  const QString &_email);
private slots:
    void slotConnected();
    void readData();
    void websocketMessageReceived(const QByteArray &message);
    void slotSocketError(QAbstractSocket::SocketError error);
    void slotWebSocketError(QAbstractSocket::SocketError error);
    void ping();
    void processServerIdentificationEvent(const Event_ServerIdentification &event);
    void processConnectionClosedEvent(const Event_ConnectionClosed &event);
    void passwordSaltResponse(const Response &response);
    void loginResponse(const Response &response);
    void registerResponse(const Response &response);
    void activateResponse(const Response &response);
    void
    doConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void doRegisterToServer(const QString &hostname,
                            unsigned int port,
                            const QString &_userName,
                            const QString &_password,
                            const QString &_email,
                            const QString &_country,
                            const QString &_realname);
    void doRequestPasswordSalt();
    void doLogin();
    void doHashedLogin();
    Command_Login generateCommandLogin();
    void doDisconnectFromServer();
    void doActivateToServer(const QString &_token);
    void doRequestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName);
    void requestForgotPasswordResponse(const Response &response);
    void doSubmitForgotPasswordResetToServer(const QString &hostname,
                                             unsigned int port,
                                             const QString &_userName,
                                             const QString &_token,
                                             const QString &_newpassword);
    void submitForgotPasswordResetResponse(const Response &response);
    void doSubmitForgotPasswordChallengeToServer(const QString &hostname,
                                                 unsigned int port,
                                                 const QString &_userName,
                                                 const QString &_email);
    void submitForgotPasswordChallengeResponse(const Response &response);

private:
    INetworkSettingsProvider *networkSettingsProvider;
    int maxTimeout;
    int timeRunning, lastDataReceived;
    QByteArray inputBuffer;
    bool messageInProgress;
    bool handshakeStarted;
    bool usingWebSocket;
    int messageLength;
    QTimer *timer;
    QTcpSocket *socket;
    QWebSocket *websocket;
    QString lastHostname;
    unsigned int lastPort;
    QString hashedPassword;

    QString getSrvClientID(const QString &_hostname);
    bool newMissingFeatureFound(const QString &_serversMissingFeatures);
    void clearNewClientFeatures();
    void connectToHost(const QString &hostname, unsigned int port);

protected slots:
    void sendCommandContainer(const CommandContainer &cont) override;

public:
    explicit RemoteClient(QObject *parent = nullptr, INetworkSettingsProvider *networkSettingsProvider = nullptr);
    ~RemoteClient() override;
    QString peerName() const
    {
        if (usingWebSocket) {
            return websocket->peerName();
        } else {
            return socket->peerName();
        }
    }
    void
    connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void registerToServer(const QString &hostname,
                          unsigned int port,
                          const QString &_userName,
                          const QString &_password,
                          const QString &_email,
                          const QString &_country,
                          const QString &_realname);
    void activateToServer(const QString &_token);
    void disconnectFromServer();
    void requestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName);
    void submitForgotPasswordResetToServer(const QString &hostname,
                                           unsigned int port,
                                           const QString &_userName,
                                           const QString &_token,
                                           const QString &_newpassword);
    void submitForgotPasswordChallengeToServer(const QString &hostname,
                                               unsigned int port,
                                               const QString &_userName,
                                               const QString &_email);
};

#endif
