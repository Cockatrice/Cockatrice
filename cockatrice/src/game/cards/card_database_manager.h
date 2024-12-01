
#ifndef CARD_DATABASE_ACCESSOR_H
#define CARD_DATABASE_ACCESSOR_H

#pragma once
#include "card_database.h"

class CardDatabaseManager
{
public:
    // Delete copy constructor and assignment operator to enforce singleton
    CardDatabaseManager(const CardDatabaseManager &) = delete;
    CardDatabaseManager &operator=(const CardDatabaseManager &) = delete;

    // Static method to access the singleton instance
    static CardDatabase *getInstance();

private:
    CardDatabaseManager() = default; // Private constructor
    ~CardDatabaseManager() = default;
};

#endif // CARD_DATABASE_ACCESSOR_H
