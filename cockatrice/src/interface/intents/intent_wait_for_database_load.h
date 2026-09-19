#ifndef COCKATRICE_INTENT_WAIT_FOR_DATABASE_LOAD_H
#define COCKATRICE_INTENT_WAIT_FOR_DATABASE_LOAD_H

#include "intent.h"

class IntentWaitForDatabaseLoad : public Intent
{
    Q_OBJECT

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;
};

#endif // COCKATRICE_INTENT_WAIT_FOR_DATABASE_LOAD_H
