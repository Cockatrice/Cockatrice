#ifndef COCKATRICE_CARD_DATABASE_QUERIER_H
#define COCKATRICE_CARD_DATABASE_QUERIER_H

#include "../card_info.h"
#include "../printing/exact_card.h"

#include <QObject>
#include <libcockatrice/interfaces/interface_card_preference_provider.h>
#include <libcockatrice/utility/card_ref.h>

class CardDatabase;

/**
 * @class CardDatabaseQuerier
 * @ingroup CardDatabase
 * @brief Provides lookup and convenience functions for querying cards and their printings.
 *
 * The CardDatabaseQuerier class offers various lookup helpers for retrieving card information
 * (e.g., CardInfoPtr, ExactCard, and PrintingInfo) from a CardDatabase. It also applies user
 * printing preferences via ICardPreferenceProvider when determining preferred printings.
 */
class CardDatabaseQuerier : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a CardDatabaseQuerier.
     *
     * @param parent Parent QObject.
     * @param db Pointer to the CardDatabase used for lookups.
     * @param prefs Pointer to card preference provider which supplies user-preference for printings.
     */
    explicit CardDatabaseQuerier(QObject *parent, const CardDatabase *db, const ICardPreferenceProvider *prefs);

    /**
     * @brief Retrieves a card by its exact name.
     *
     * @param cardName Exact card name.
     * @return A CardInfoPtr, or null if no matching card exists.
     */
    [[nodiscard]] CardInfoPtr getCardInfo(const QString &cardName) const;

    /**
     * @brief Retrieves multiple cards by their exact names.
     *
     * Failed lookups are skipped and not included in the result.
     *
     * @param cardNames List of exact card names.
     * @return List of CardInfoPtr objects for which a match was found.
     */
    [[nodiscard]] QList<CardInfoPtr> getCardInfos(const QStringList &cardNames) const;

    /**
     * @brief Retrieves a card using simplified name matching.
     *
     * The name is automatically normalized, so callers do not need to simplify it.
     *
     * @param cardName A (possibly simplified or misspelled) card name.
     * @return A CardInfoPtr, or null if not found.
     */
    [[nodiscard]] CardInfoPtr getCardBySimpleName(const QString &cardName) const;

    /**
     * @brief Looks up a card using exact name first, then simplified matching as fallback.
     *
     * @param name Raw card name input.
     * @return The best-match CardInfoPtr, or null if no match is found.
     */
    [[nodiscard]] CardInfoPtr lookupCardByName(const QString &name) const;

    /**
     * @brief Converts a CardRef into an ExactCard.
     *
     * If the providerId is empty, the preferred printing is used.
     * If providerId exists but cannot be found, an ExactCard with an empty PrintingInfo is returned.
     *
     * @param cardRef Card reference with name and optional providerId.
     * @return The resolved ExactCard, or empty if no card was found.
     */
    [[nodiscard]] ExactCard getCard(const CardRef &cardRef) const;

    /**
     * @brief Resolves multiple CardRefs into ExactCards.
     *
     * Failed entries are not included in the result.
     *
     * @param cardRefs List of card references.
     * @return List of successfully resolved ExactCards.
     */
    [[nodiscard]] QList<ExactCard> getCards(const QList<CardRef> &cardRefs) const;

    /**
     * @brief Attempts a more flexible card lookup using both simple name matching and CardRef rules.
     *
     * If providerId is missing, uses preferred printing. If lookup fails, attempts simplified name.
     *
     * @param cardRef Card reference to resolve.
     * @return The best-guess ExactCard, or empty if unresolved.
     */
    [[nodiscard]] ExactCard guessCard(const CardRef &cardRef) const;

    /**
     * @brief Returns a random card from the database using the preferred printing.
     *
     * @return A random ExactCard, or empty if the database is empty.
     */
    [[nodiscard]] ExactCard getRandomCard() const;

    /**
     * @brief Returns a printing of a card from the same set as another given printing when possible.
     *
     * If no matching printing exists, falls back to a standard lookup.
     *
     * @param cardName Card to retrieve.
     * @param otherPrinting Printing to match the set against.
     * @return Matching ExactCard if found, otherwise fallback ExactCard.
     */
    [[nodiscard]] ExactCard getCardFromSameSet(const QString &cardName, const PrintingInfo &otherPrinting) const;

    /**
     * @brief Returns the preferred printing of a card based on user preferences and set priority.
     *
     * @param cardName Name of the card.
     * @return The preferred ExactCard.
     */
    [[nodiscard]] ExactCard getPreferredCard(const QString &cardName) const;

    /**
     * @brief Returns the preferred printing of a card based on user preferences and set priority.
     *
     * @param cardInfo Card information object.
     * @return The preferred ExactCard.
     */
    [[nodiscard]] ExactCard getPreferredCard(const CardInfoPtr &cardInfo) const;

    /**
     * @brief Checks whether the CardRef refers to the preferred printing.
     *
     * @param cardRef Card reference to test.
     * @return True if providerId matches the preferred printing.
     */
    [[nodiscard]] bool isPreferredPrinting(const CardRef &cardRef) const;

    /**
     * @brief Returns the preferred printing for the given card name.
     *
     * @param cardName Card name.
     * @return Preferred PrintingInfo, or empty if not found.
     */
    [[nodiscard]] PrintingInfo getPreferredPrinting(const QString &cardName) const;

    /**
     * @brief Returns the preferred printing for the given card.
     *
     * @param cardInfo Card information object.
     * @return Preferred PrintingInfo, or empty if not applicable.
     */
    [[nodiscard]] PrintingInfo getPreferredPrinting(const CardInfoPtr &cardInfo) const;

    /**
     * @brief Returns the providerId of the preferred printing.
     *
     * @param cardName Card name.
     * @return ProviderId string for preferred printing.
     */
    [[nodiscard]] QString getPreferredPrintingProviderId(const QString &cardName) const;

    /**
     * @brief Retrieves a specific printing referenced by CardRef.
     *
     * @param cardRef Card reference including providerId.
     * @return Matching PrintingInfo, or empty if not found.
     */
    [[nodiscard]] PrintingInfo getSpecificPrinting(const CardRef &cardRef) const;

    /**
     * @brief Searches for a specific printing by set code and collector number.
     *
     * @param cardName Card name to search.
     * @param setCode Set (short) code to match.
     * @param collectorNumber Collector number. If empty, any printing from the set is returned.
     * @return Matching PrintingInfo, or empty if not found.
     */
    [[nodiscard]] PrintingInfo
    getSpecificPrinting(const QString &cardName, const QString &setCode, const QString &collectorNumber) const;

    /**
     * @brief Searches for a printing that matches a given providerId.
     *
     * @param card Card to search.
     * @param providerId Provider identifier to match.
     * @return Matching PrintingInfo, or empty if not found.
     */
    [[nodiscard]] PrintingInfo findPrintingWithId(const CardInfoPtr &card, const QString &providerId) const;

    /**
     * @brief Returns a list of all main card types present in the database.
     *
     * @return List of main card type strings.
     */
    [[nodiscard]] QStringList getAllMainCardTypes() const;

    /**
     * @brief Returns a mapping of main card types to their occurrence counts.
     *
     * @return Map of main card type to count.
     */
    [[nodiscard]] QMap<QString, int> getAllMainCardTypesWithCount() const;

    /**
     * @brief Returns a mapping of card subtypes to their occurrence counts.
     *
     * @return Map of subtype string to count.
     */
    [[nodiscard]] QMap<QString, int> getAllSubCardTypesWithCount() const;
    FormatRulesPtr getFormat(const QString &formatName) const;
    QMap<QString, int> getAllFormatsWithCount() const;

private:
    const CardDatabase *db;               //!< Card database used for all lookups.
    const ICardPreferenceProvider *prefs; //!< Preference provider for preferred printings.
};

#endif // COCKATRICE_CARD_DATABASE_QUERIER_H
