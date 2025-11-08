#ifndef COCKATRICE_SETTINGS_CARD_PREFERENCE_PROVIDER_H
#define COCKATRICE_SETTINGS_CARD_PREFERENCE_PROVIDER_H
#include "../../client/settings/cache_settings.h"

#include <libcockatrice/interfaces/interface_card_preference_provider.h>

class SettingsCardPreferenceProvider : public ICardPreferenceProvider
{
public:
    QString getCardPreferenceOverride(const QString &cardName) const override
    {
        return SettingsCache::instance().cardOverrides().getCardPreferenceOverride(cardName);
    }

    bool getIncludeRebalancedCards() const override
    {
        return SettingsCache::instance().getIncludeRebalancedCards();
    };
};

#endif // COCKATRICE_SETTINGS_CARD_PREFERENCE_PROVIDER_H
