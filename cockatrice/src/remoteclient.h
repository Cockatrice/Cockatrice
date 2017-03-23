#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QTcpSocket>
#include "abstractclient.h"

class QTimer;

class RemoteClient : public AbstractClient {
    Q_OBJECT
signals:
    void maxPingTime(int seconds, int maxSeconds);
    void serverTimeout();
    void loginError(Response::ResponseCode resp, QString reasonStr, quint32 endTime, QList<QString> missingFeatures);
    void registerError(Response::ResponseCode resp, QString reasonStr, quint32 endTime);
    void activateError();
    void socketError(const QString &errorString);
    void protocolVersionMismatch(int clientVersion, int serverVersion);
    void protocolError();
    void sigConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void sigRegisterToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password, const QString &_email, const int _gender, const QString &_country, const QString &_realname);
    void sigActivateToServer(const QString &_token);
    void sigDisconnectFromServer();
    void notifyUserAboutUpdate();
    void sigRequestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName);
    void sigForgotPasswordSuccess();
    void sigForgotPasswordError();
    void sigPromptForForgotPasswordReset();
    void sigSubmitForgotPasswordResetToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_token, const QString &_newpassword);
    void sigPromptForForgotPasswordChallenge();
    void sigSubmitForgotPasswordChallengeToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_email);
private slots:
    void slotConnected();
    void readData();
    void slotSocketError(QAbstractSocket::SocketError error);
    void ping();
    void processServerIdentificationEvent(const Event_ServerIdentification &event);
    void processConnectionClosedEvent(const Event_ConnectionClosed &event);
    void loginResponse(const Response &response);
    void registerResponse(const Response &response);
    void activateResponse(const Response &response);
    void doConnectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void doRegisterToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password, const QString &_email, const int _gender, const QString &_country, const QString &_realname);
    void doLogin();
    void doDisconnectFromServer();
    void doActivateToServer(const QString &_token);
    void doRequestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName);
    void requestForgotPasswordResponse(const Response &response);
    void doSubmitForgotPasswordResetToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_token, const QString &_newpassword);
    void submitForgotPasswordResetResponse(const Response &response);
    void doSubmitForgotPasswordChallengeToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_email);
    void submitForgotPasswordChallengeResponse(const Response &response);
private:
    static const int maxTimeout = 10;
    int timeRunning, lastDataReceived;

    QByteArray inputBuffer;
    bool messageInProgress;
    bool handshakeStarted;
    bool newMissingFeatureFound(QString _serversMissingFeatures);
    void clearNewClientFeatures();
    int messageLength;
    
    QTimer *timer;
    QTcpSocket *socket;
    QString lastHostname;
    int lastPort;
    QString getSrvClientID(const QString _hostname);
protected slots:    
    void sendCommandContainer(const CommandContainer &cont);
public:
    RemoteClient(QObject *parent = 0);
    ~RemoteClient();
    QString peerName() const { return socket->peerName(); }
    void connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
    void registerToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password, const QString &_email, const int _gender, const QString &_country, const QString &_realname);
    void activateToServer(const QString &_token);
    void disconnectFromServer();
    void requestForgotPasswordToServer(const QString &hostname, unsigned int port, const QString &_userName);
    void submitForgotPasswordResetToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_token, const QString &_newpassword);
    void submitForgotPasswordChallengeToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_email);
};

#endif
