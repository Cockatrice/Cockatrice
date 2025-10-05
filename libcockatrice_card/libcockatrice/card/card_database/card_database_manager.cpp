#include "card_database_manager.h"

CardDatabase *CardDatabaseManager::getInstance()
{
    static CardDatabase instance; // Created only once, on first access
    return &instance;
}

CardDatabaseQuerier *CardDatabaseManager::query()
{
    return getInstance()->query();
}