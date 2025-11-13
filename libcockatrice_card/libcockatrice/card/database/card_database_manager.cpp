#include "card_database_manager.h"

#include <libcockatrice/interfaces/noop_card_database_path_provider.h>
#include <libcockatrice/interfaces/noop_card_preference_provider.h>
#include <libcockatrice/interfaces/noop_card_set_priority_controller.h>

ICardPreferenceProvider *CardDatabaseManager::cardPreferenceProvider = new NoopCardPreferenceProvider();
ICardDatabasePathProvider *CardDatabaseManager::pathProvider = new NoopCardDatabasePathProvider();
ICardSetPriorityController *CardDatabaseManager::setPriorityController = new NoopCardSetPriorityController();

void CardDatabaseManager::setCardPreferenceProvider(ICardPreferenceProvider *provider)
{
    cardPreferenceProvider = provider;
}

void CardDatabaseManager::setCardDatabasePathProvider(ICardDatabasePathProvider *provider)
{
    pathProvider = provider;
}

void CardDatabaseManager::setCardSetPriorityController(ICardSetPriorityController *controller)
{
    setPriorityController = controller;
}

CardDatabase *CardDatabaseManager::getInstance()
{
    static CardDatabase instance(nullptr, cardPreferenceProvider, pathProvider, setPriorityController);
    return &instance;
}

CardDatabaseQuerier *CardDatabaseManager::query()
{
    return getInstance()->query();
}