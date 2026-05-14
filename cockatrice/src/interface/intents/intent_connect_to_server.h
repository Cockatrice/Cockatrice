#ifndef INTENT_CONNECT_TO_SERVER_H
#define INTENT_CONNECT_TO_SERVER_H

#include "contexts/context_connect_to_server.h"

#include <libcockatrice/utility/intent.h>

class ConnectionController;
class QWidget;

/**
 * @brief Resolves credentials for the URL's target server, then fires
 *        connectToServerDirect() and emits finished(true).
 *
 * Resolution order:
 *   1. Look up saved credentials in ServersSettings by hostname+port.  If a
 *      match with a saved password is found, use them.
 *   2. Otherwise, open DlgConnect pre-filled with the URL's hostname/port so
 *      the user can enter credentials.  finished(true) when the user clicks
 *      Connect; finished(false) when they cancel.
 *
 * The dialog path is user-paced — overriding @c timeoutMs() to return @c -1
 * disables the chain's timeout safety net for this intent (the dialog can
 * stay open arbitrarily long).  The follow-on @c IntentLogin still has its
 * own 30s deadline on the actual login round-trip.
 *
 * The actual login success/failure is detected by the following IntentLogin
 * in the chain.
 */
class IntentConnectToServer : public Intent
{
    Q_OBJECT
public:
    explicit IntentConnectToServer(const ContextConnectToServer &ctx,
                                   ConnectionController *controller,
                                   QWidget *dialogParent,
                                   QObject *parent = nullptr);

    [[nodiscard]] int timeoutMs() const override
    {
        return -1; // user-paced via DlgConnect when no saved creds match
    }

protected:
    void doExecute() override;

private:
    ContextConnectToServer ctx;
    ConnectionController *controller;
    QWidget *dialogParent;
};

#endif // INTENT_CONNECT_TO_SERVER_H
