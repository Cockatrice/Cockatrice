#ifndef INTENT_LOGIN_H
#define INTENT_LOGIN_H

#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/utility/intent.h>

class ConnectionController;

/**
 * @brief Waits for the server login to complete after a connection attempt.
 *
 * Connects to ConnectionController::statusChanged and emits finished(true)
 * when StatusLoggedIn is reached, or finished(false) when StatusDisconnected
 * is reached.  Short-circuits to finished(false) when the controller is
 * already disconnected at execute() time (the upstream connect step failed
 * synchronously), and gives up after a 30-second timeout if the server
 * accepts the connection but never sends a login response.
 */
class IntentLogin : public Intent
{
    Q_OBJECT
public:
    explicit IntentLogin(ConnectionController *controller, QObject *parent = nullptr);

protected:
    void doExecute() override;

private:
    ConnectionController *controller;
};

#endif // INTENT_LOGIN_H
