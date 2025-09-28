#include "card_database_manager.h"

CardDatabase *CardDatabaseManager::getInstance()
{
    static CardDatabase instance(
        nullptr, std::make_shared<SettingsCardPreferenceProvider>()); // Created only once, on first access
    return &instance;
}

CardDatabaseQuerier *CardDatabaseManager::query()
{
    return getInstance()->query();
}