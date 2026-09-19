#ifndef COCKATRICE_INTENT_DISCONNECT_FROM_SERVER_H
#define COCKATRICE_INTENT_DISCONNECT_FROM_SERVER_H

#include "intent.h"
#include "remote_client.h"

class IntentDisconnectFromServer : public Intent
{
    Q_OBJECT

public:
    IntentDisconnectFromServer(RemoteClient *_remoteClient);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    RemoteClient *remoteClient;

private slots:
    void onStatusChanged(ClientStatus status);
};

#endif // COCKATRICE_INTENT_DISCONNECT_FROM_SERVER_H
