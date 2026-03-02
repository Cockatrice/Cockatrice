#include "card_override_settings.h"

CardOverrideSettings::CardOverrideSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "cardPreferenceOverrides.ini", "cards", QString(), parent)
{
}

void CardOverrideSettings::setCardPreferenceOverride(const CardRef &cardRef)
{
    setValue(cardRef.providerId, cardRef.name);
}

void CardOverrideSettings::deleteCardPreferenceOverride(const QString &cardName)
{
    deleteValue(cardName);
}

QString CardOverrideSettings::getCardPreferenceOverride(const QString &cardName)
{
    return getValue(cardName).toString();
}