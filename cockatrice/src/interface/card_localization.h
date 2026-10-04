#ifndef COCKATRICE_CARD_LOCALIZATION_H
#define COCKATRICE_CARD_LOCALIZATION_H

#include "../client/settings/cache_settings.h"

#include <QString>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/settings/cards_display_settings.h>

namespace CardLocalization
{
/**
 * @brief The language code selected for localized card text and images.
 */
inline QString displayLang()
{
    return SettingsCache::instance().cardsDisplay().getCardLang();
}

/**
 * @brief Card name in the configured display language, falling back to English.
 * @param card The card to display.
 * @return The localized name, or an empty string for a null card.
 */
inline QString displayName(const CardInfoPtr &card)
{
    return card.isNull() ? QString() : card->getLocalizedName(displayLang());
}

/**
 * @brief Card rules text in the configured display language, falling back to English.
 * @param card The card to display.
 * @return The localized text, or an empty string for a null card.
 */
inline QString displayText(const CardInfoPtr &card)
{
    return card.isNull() ? QString() : card->getLocalizedText(displayLang());
}

/**
 * @brief Card name in the configured display language, falling back to English.
 * @param card The card to display.
 */
inline QString displayName(const CardInfo &card)
{
    return card.getLocalizedName(displayLang());
}

/**
 * @brief Card rules text in the configured display language, falling back to English.
 * @param card The card to display.
 */
inline QString displayText(const CardInfo &card)
{
    return card.getLocalizedText(displayLang());
}

/**
 * @brief Looks a card up by its canonical name and returns its name in the configured display language.
 *
 * Use this where only the canonical name is known, such as the game log or [card] chat tags. Callers that
 * already hold a resolved card and need to keep the canonical name around (as a link target, say) should
 * use displayName() directly instead.
 *
 * @param cardName The canonical card name.
 * @return The localized name, or cardName unchanged when the card is unknown to the database.
 */
inline QString displayNameFor(const QString &cardName)
{
    if (cardName.isEmpty()) {
        return cardName;
    }
    const QString localizedName = displayName(CardDatabaseManager::query()->getCardInfo(cardName));
    return localizedName.isEmpty() ? cardName : localizedName;
}
} // namespace CardLocalization

#endif // COCKATRICE_CARD_LOCALIZATION_H