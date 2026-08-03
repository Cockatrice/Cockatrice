#include "intent_open_local_deck.h"

#include "../deck_loader/deck_file_format.h"
#include "../deck_loader/deck_loader.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_wait_for_database_load.h"

#include <libcockatrice/card/database/card_database_manager.h>

IntentOpenLocalDeck::IntentOpenLocalDeck(TabSupervisor *_tabSupervisor, const QString &_file)
    : Intent(), tabSupervisor(_tabSupervisor), file(_file)
{
}

bool IntentOpenLocalDeck::checkPrecondition() const
{
    return CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok;
}

void IntentOpenLocalDeck::onPreconditionSatisfied()
{
    std::optional<LoadedDeck> deckOpt = DeckLoader::loadFromFile(file, DeckFileFormat::getFormatFromName(file), true);
    if (deckOpt) {
        tabSupervisor->openDeckInNewTab(deckOpt.value());
    }
    emit finished();
}

void IntentOpenLocalDeck::onPreconditionNotSatisfied()
{
    runDependency(new IntentWaitForDatabaseLoad);
}
