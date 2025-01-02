#include "card_override_settings.h"

CardOverrideSettings::CardOverrideSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "cardPreferenceOverrides.ini", parent)
{
}

void CardOverrideSettings::setCardPreferenceOverride(const QString &cardName, const QString &providerId)
{
    setValue(providerId, cardName, "cards");
}

void CardOverrideSettings::deleteCardPreferenceOverride(const QString &cardName)
{
    deleteValue(cardName, "cards");
}

QString CardOverrideSettings::getCardPreferenceOverride(const QString &cardName)
{
    return getValue(cardName, "cards").toString();
}