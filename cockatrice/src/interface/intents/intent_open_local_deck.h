#ifndef COCKATRICE_INTENT_OPEN_LOCAL_DECK_H
#define COCKATRICE_INTENT_OPEN_LOCAL_DECK_H
#include "../widgets/tabs/tab_supervisor.h"
#include "intent.h"
#include "intent_wait_for_database_load.h"
#include "libcockatrice/card/database/card_database_manager.h"

class IntentOpenLocalDeck : public Intent
{
    Q_OBJECT

public:
    IntentOpenLocalDeck(TabSupervisor *_tabSupervisor, const QString &_file)
        : Intent(), tabSupervisor(_tabSupervisor), file(_file)
    {
    }

protected:
    bool checkPrecondition() const override
    {
        return CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok;
    }

    void onPreconditionSatisfied() override
    {
        std::optional<LoadedDeck> deckOpt =
            DeckLoader::loadFromFile(file, DeckFileFormat::getFormatFromName(file), true);
        if (deckOpt) {
            tabSupervisor->openDeckInNewTab(deckOpt.value());
        }
        emit finished();
    }

    void onPreconditionNotSatisfied() override
    {
        runDependency(new IntentWaitForDatabaseLoad);
    }

private:
    TabSupervisor *tabSupervisor;
    const QString &file;
};

#endif // COCKATRICE_INTENT_OPEN_LOCAL_DECK_H
