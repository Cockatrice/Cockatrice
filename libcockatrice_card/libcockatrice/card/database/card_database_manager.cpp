#include "card_database_manager.h"

QSharedPointer<ICardPreferenceProvider> CardDatabaseManager::cardPreferenceProvider =
    QSharedPointer<NoopCardPreferenceProvider>::create();

void CardDatabaseManager::setCardPreferenceProvider(QSharedPointer<ICardPreferenceProvider> provider)
{
    cardPreferenceProvider = provider;
}

CardDatabase *CardDatabaseManager::getInstance()
{
    static CardDatabase instance(nullptr, cardPreferenceProvider);
    return &instance;
}

CardDatabaseQuerier *CardDatabaseManager::query()
{
    return getInstance()->query();
}