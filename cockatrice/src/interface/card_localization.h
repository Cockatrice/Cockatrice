#ifndef COCKATRICE_CARD_LOCALIZATION_H
#define COCKATRICE_CARD_LOCALIZATION_H

#include "../client/settings/cache_settings.h"

#include <QString>
#include <libcockatrice/card/card_info.h>
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
} // namespace CardLocalization

#endif // COCKATRICE_CARD_LOCALIZATION_H