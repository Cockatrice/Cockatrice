#include "card_database_manager.h"

#include "interface/noop_card_database_path_provider.h"
#include "interface/noop_card_set_priority_controller.h"

QSharedPointer<ICardPreferenceProvider> CardDatabaseManager::cardPreferenceProvider =
    QSharedPointer<NoopCardPreferenceProvider>::create();
QSharedPointer<ICardDatabasePathProvider> CardDatabaseManager::pathProvider =
    QSharedPointer<NoopCardDatabasePathProvider>::create();
QSharedPointer<ICardSetPriorityController> CardDatabaseManager::setPriorityController =
    QSharedPointer<NoopCardSetPriorityController>::create();

void CardDatabaseManager::setCardPreferenceProvider(QSharedPointer<ICardPreferenceProvider> provider)
{
    cardPreferenceProvider = provider;
}

void CardDatabaseManager::setCardDatabasePathProvider(QSharedPointer<ICardDatabasePathProvider> provider)
{
    pathProvider = provider;
}

void CardDatabaseManager::setCardSetPriorityController(QSharedPointer<ICardSetPriorityController> controller)
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