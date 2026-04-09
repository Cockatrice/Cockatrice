#ifndef COCKATRICE_REMOTE_CONNECTION_CONTROLLER_H
#define COCKATRICE_REMOTE_CONNECTION_CONTROLLER_H

#pragma once

#include "abstract_client.h"

#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QThread>
#include <libcockatrice/protocol/pb/event_connection_closed.pb.h>
#include <libcockatrice/protocol/pb/event_server_shutdown.pb.h>

class RemoteClient;
class ServerInfo_User;
class DlgConnect;

/**
 * Owns the RemoteClient and its worker thread.
 * Encapsulates all connection, authentication, and registration logic so that
 * MainWindow only needs to react to high-level signals.
 */
class ConnectionController : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionController(QWidget *dialogParent, QObject *parent = nullptr);
    ~ConnectionController() override;

    RemoteClient *client() const
    {
        return remoteClient;
    }

    void registerToServer();
    void forgotPasswordRequest();
    void connectToServer();
    void
    connectToServerDirect(const QString &host, unsigned int port, const QString &playerName, const QString &password);
    void disconnectFromServer();

    void refreshWindowTitle()
    {
        updateWindowTitle();
    }

signals:
    void windowTitleChanged(const QString &title);

    void tabSupervisorStartRequested(const ServerInfo_User &info);
    void tabSupervisorStopRequested();

    // Passes the raw ClientStatus through so MainWindow can drive its own
    // action enable/disable logic
    void statusChanged(ClientStatus status);

private slots:
    // Slots wired directly to RemoteClient signals
    void onStatusChanged(ClientStatus status);
    void onUserInfoReceived(const ServerInfo_User &info);
    void onLoginError(int r, QString reasonStr, quint32 endTime, const QList<QString> &missingFeatures);
    void onRegisterAccepted();
    void onRegisterAcceptedNeedsActivate();
    void onRegisterError(int r, QString reasonStr, quint32 endTime);
    void onActivateAccepted();
    void onActivateError();
    void onProtocolVersionMismatch(int localVersion, int remoteVersion);
    void onNotifyUserAboutUpdate();
    void onConnectionClosedEvent(const Event_ConnectionClosed &event);
    void onServerShutdownEvent(const Event_ServerShutdown &event);
    void onSocketError(const QString &errorStr);
    void onServerTimeout();

    // Forgot-password flow
    void onForgotPasswordSuccess();
    void onForgotPasswordError();
    void onPromptForgotPasswordReset();
    void onPromptForgotPasswordChallenge();

private:
    void wireClientSignals();
    void updateWindowTitle();

    /** Parse the server's pipe-delimited username-rule string into HTML. */
    static QString extractInvalidUsernameMessage(QString &in);

    RemoteClient *remoteClient{nullptr};
    QThread *clientThread{nullptr};
    QWidget *dialogParent{nullptr}; // used as parent for QMessageBox / dialog calls

    // Persistent so it can be updated in-place by onServerShutdownEvent
    QMessageBox serverShutdownMessageBox;

    // Kept as a member so the forgot-password signal can be wired to it
    DlgConnect *dlgConnect{nullptr};
};

#endif // COCKATRICE_REMOTE_CONNECTION_CONTROLLER_H
