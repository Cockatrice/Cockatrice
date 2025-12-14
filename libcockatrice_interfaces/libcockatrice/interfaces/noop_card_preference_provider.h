#ifndef COCKATRICE_NOOP_CARD_PREFERENCE_PROVIDER_H
#define COCKATRICE_NOOP_CARD_PREFERENCE_PROVIDER_H
#include "interface_card_preference_provider.h"

class NoopCardPreferenceProvider : public ICardPreferenceProvider
{
public:
    [[nodiscard]] QString getCardPreferenceOverride(const QString &) const override
    {
        return {};
    }

    [[nodiscard]] bool getIncludeRebalancedCards() const override
    {
        return true;
    }
};

#endif // COCKATRICE_NOOP_CARD_PREFERENCE_PROVIDER_H
