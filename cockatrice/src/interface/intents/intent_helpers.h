#ifndef INTENT_HELPERS_H
#define INTENT_HELPERS_H

#include "../../client/network/connection_controller/remote_connection_controller.h"

#include <QObject>
#include <libcockatrice/utility/intent.h>

/**
 * @brief Wire @p intent to abort when the @p controller's connection drops.
 *
 * Shared boilerplate for intents that wait on a network round-trip and must
 * not hang if the user (or server) tears the connection down mid-flight.
 */
inline void abortOnDisconnect(Intent *intent, ConnectionController *controller)
{
    QObject::connect(controller, &ConnectionController::statusChanged, intent, [intent](ClientStatus status) {
        if (status == StatusDisconnected)
            intent->abort();
    });
}

#endif // INTENT_HELPERS_H
