/**
 * @file card_override_settings.h
 * @ingroup CardSettings
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_CARD_OVERRIDE_SETTINGS_H
#define COCKATRICE_CARD_OVERRIDE_SETTINGS_H

#include "settings_manager.h"

#include <QObject>
#include <libcockatrice/utility/card_ref.h>

class CardOverrideSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    void setCardPreferenceOverride(const CardRef &cardRef);

    void deleteCardPreferenceOverride(const QString &cardName);

    QString getCardPreferenceOverride(const QString &cardName);

private:
    explicit CardOverrideSettings(const QString &settingPath, QObject *parent = nullptr);
    CardOverrideSettings(const CardOverrideSettings & /*other*/);
};

#endif // COCKATRICE_CARD_OVERRIDE_SETTINGS_H
