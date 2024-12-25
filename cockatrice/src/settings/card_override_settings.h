#ifndef COCKATRICE_CARD_OVERRIDE_SETTINGS_H
#define COCKATRICE_CARD_OVERRIDE_SETTINGS_H

#include "settings_manager.h"

#include <QObject>

class CardOverrideSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    void setCardPreferenceOverride(QString cardName, QString providerId, bool enabled);

    QString getCardPreferenceOverride(QString cardName);

private:
    explicit CardOverrideSettings(QString settingPath, QObject *parent = nullptr);
    CardOverrideSettings(const CardOverrideSettings & /*other*/);
};

#endif // COCKATRICE_CARD_OVERRIDE_SETTINGS_H
