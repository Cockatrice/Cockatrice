#ifndef COCKATRICE_INTERFACE_CARD_PREFERENCE_PROVIDER_H
#define COCKATRICE_INTERFACE_CARD_PREFERENCE_PROVIDER_H

#include <QString>

class ICardPreferenceProvider
{
public:
    virtual ~ICardPreferenceProvider() = default;
    [[nodiscard]] virtual QString getCardPreferenceOverride(const QString &cardName) const = 0;
    [[nodiscard]] virtual bool getIncludeRebalancedCards() const = 0;
};

#endif // COCKATRICE_INTERFACE_CARD_PREFERENCE_PROVIDER_H
