#ifndef CARD_DATABASE_ACCESSOR_H
#define CARD_DATABASE_ACCESSOR_H

#pragma once
#include "card_database.h"

/**
 * @class CardDatabaseManager
 * @ingroup CardDatabase
 * @brief The CardDatabaseManager is responsible for managing the global CardDatabase singleton.
 *
 * This class provides a static interface for accessing the global CardDatabase instance
 * and its CardDatabaseQuerier. It also allows the configuration of optional providers:
 *  - ICardPreferenceProvider
 *  - ICardDatabasePathProvider
 *  - ICardSetPriorityController
 *
 * Only a single instance of CardDatabase exists, enforced via a private constructor and
 * deleted copy/move operations.
 */
class CardDatabaseManager
{
public:
    /** @brief Deleted copy constructor to enforce singleton. */
    CardDatabaseManager(const CardDatabaseManager &) = delete;

    /** @brief Deleted assignment operator to enforce singleton. */
    CardDatabaseManager &operator=(const CardDatabaseManager &) = delete;

    /**
     * @brief Sets the card preference provider.
     * @param provider Pointer to an ICardPreferenceProvider.
     * @note Must be called before the first call to getInstance().
     */
    static void setCardPreferenceProvider(ICardPreferenceProvider *provider);

    /**
     * @brief Sets the card database path provider.
     * @param provider Pointer to an ICardDatabasePathProvider.
     * @note Must be called before the first call to getInstance().
     */
    static void setCardDatabasePathProvider(ICardDatabasePathProvider *provider);

    /**
     * @brief Sets the card set priority controller.
     * @param controller Pointer to an ICardSetPriorityController.
     * @note Must be called before the first call to getInstance().
     */
    static void setCardSetPriorityController(ICardSetPriorityController *controller);

    /**
     * @brief Returns the singleton CardDatabase instance.
     * @return Pointer to the global CardDatabase.
     */
    static CardDatabase *getInstance();

    /**
     * @brief Returns the CardDatabaseQuerier of the singleton database.
     * @return Pointer to CardDatabaseQuerier.
     */
    static CardDatabaseQuerier *query();

private:
    /** @brief Private default constructor to enforce singleton. */
    CardDatabaseManager() = default;

    /** @brief Private destructor. */
    ~CardDatabaseManager() = default;

    /// Static card preference provider pointer (default: Noop)
    static ICardPreferenceProvider *cardPreferenceProvider;

    /// Static path provider pointer (default: Noop)
    static ICardDatabasePathProvider *pathProvider;

    /// Static set priority controller pointer (default: Noop)
    static ICardSetPriorityController *setPriorityController;
};

#endif // CARD_DATABASE_ACCESSOR_H
