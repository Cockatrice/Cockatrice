#include "intent_login.h"

#include "../../client/network/connection_controller/remote_connection_controller.h"

#include <QLoggingCategory>
#include <libcockatrice/network/client/remote/remote_client.h>

Q_LOGGING_CATEGORY(IntentLoginLog, "intent.login")

IntentLogin::IntentLogin(ConnectionController *controller, QObject *parent) : Intent(parent), controller(controller)
{
}

void IntentLogin::doExecute()
{
    // Quick-fail: if the controller is already disconnected when we start, the
    // upstream connect step failed synchronously and waiting for statusChanged
    // would hang.  Abort immediately.
    if (controller->client()->getStatus() == StatusDisconnected) {
        qCDebug(IntentLoginLog) << "Already disconnected at login start; aborting";
        emitFinished(false);
        return;
    }

    connect(controller, &ConnectionController::statusChanged, this, [this](ClientStatus status) {
        if (status == StatusLoggedIn) {
            qCDebug(IntentLoginLog) << "Login succeeded";
            emitFinished(true);
        } else if (status == StatusDisconnected) {
            qCDebug(IntentLoginLog) << "Connection lost before login completed";
            emitFinished(false);
        }
    });

    startTimeoutSafetyNet();
}
