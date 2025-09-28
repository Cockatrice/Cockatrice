#ifndef COCKATRICE_SETTINGS_CARD_PREFERENCE_PROVIDER_H
#define COCKATRICE_SETTINGS_CARD_PREFERENCE_PROVIDER_H
#include "../../settings/cache_settings.h"
#include "interface_card_preference_provider.h"

class SettingsCardPreferenceProvider : public ICardPreferenceProvider
{
public:
    QString getCardPreferenceOverride(const QString &cardName) const override
    {
        return SettingsCache::instance().cardOverrides().getCardPreferenceOverride(cardName);
    }
};

#endif // COCKATRICE_SETTINGS_CARD_PREFERENCE_PROVIDER_H
