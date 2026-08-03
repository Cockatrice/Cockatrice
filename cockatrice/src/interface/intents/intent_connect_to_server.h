#ifndef COCKATRICE_INTENT_CONNECT_TO_SERVER_H
#define COCKATRICE_INTENT_CONNECT_TO_SERVER_H

#include "contexts/context_connect_to_server.h"
#include "intent.h"
#include "remote_client.h"

class IntentConnectToServer : public Intent
{
    Q_OBJECT

public:
    IntentConnectToServer(RemoteClient *_remoteClient, ContextConnectToServer *_context);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    RemoteClient *remoteClient;
    ContextConnectToServer *context;

private slots:
    void onStatusChanged(ClientStatus status);
};

#endif // COCKATRICE_INTENT_CONNECT_TO_SERVER_H
