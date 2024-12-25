#include "card_override_settings.h"

CardOverrideSettings::CardOverrideSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath + "cardPreferenceOverrides.ini", parent)
{
}

void CardOverrideSettings::setCardPreferenceOverride(QString cardName, QString providerId, bool enabled)
{
    if (enabled) {
        setValue(providerId, cardName, "cards");
    } else {
        deleteValue(providerId, cardName, "cards");
    }
}

QString CardOverrideSettings::getCardPreferenceOverride(QString cardName)
{
    return getValue(cardName, "cards").toString();
}