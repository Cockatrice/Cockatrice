#ifndef COCKATRICE_INTENT_LOGIN_H
#define COCKATRICE_INTENT_LOGIN_H

#include "contexts/context_connect_to_server.h"
#include "intent.h"

class IntentGetLoginCredentials : public Intent
{
    Q_OBJECT

public:
    // When promptForMissingCredentials is false (the default) a server without
    // saved credentials fails silently; only intent chains from cockatrice://
    // links opt into the interactive sign-in dialog.
    explicit IntentGetLoginCredentials(ContextConnectToServer *_context, bool _promptForMissingCredentials = false);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    ContextConnectToServer *context;
    bool promptForMissingCredentials;
};

#endif // COCKATRICE_INTENT_LOGIN_H
