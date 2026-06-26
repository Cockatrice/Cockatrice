#ifndef COCKATRICE_INTENT_WAIT_FOR_DATABASE_LOAD_H
#define COCKATRICE_INTENT_WAIT_FOR_DATABASE_LOAD_H

#include "intent.h"
#include "libcockatrice/card/database/card_database_manager.h"

class IntentWaitForDatabaseLoad : public Intent
{
    Q_OBJECT

protected:
    bool checkPrecondition() const override
    {
        return CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok;
    }

    void onPreconditionSatisfied() override
    {
        emit finished();
    }

    void onPreconditionNotSatisfied() override
    {
        connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
                [this]() { emit finished(); });
    }
};

#endif // COCKATRICE_INTENT_WAIT_FOR_DATABASE_LOAD_H
