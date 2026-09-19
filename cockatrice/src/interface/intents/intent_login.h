#ifndef COCKATRICE_INTENT_LOGIN_H
#define COCKATRICE_INTENT_LOGIN_H

#include "contexts/context_connect_to_server.h"
#include "intent.h"

class IntentGetLoginCredentials : public Intent
{
    Q_OBJECT

public:
    IntentGetLoginCredentials(ContextConnectToServer *_context);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    ContextConnectToServer *context;
};

#endif // COCKATRICE_INTENT_LOGIN_H
