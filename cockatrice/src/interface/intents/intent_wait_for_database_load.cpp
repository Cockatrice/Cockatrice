#include "intent_wait_for_database_load.h"

#include <libcockatrice/card/database/card_database_manager.h>

bool IntentWaitForDatabaseLoad::checkPrecondition() const
{
    return CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok;
}

void IntentWaitForDatabaseLoad::onPreconditionSatisfied()
{
    emitFinished();
}

void IntentWaitForDatabaseLoad::onPreconditionNotSatisfied()
{
    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
            [this]() { emitFinished(); });
}
