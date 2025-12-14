/**
 * @file card_counter_settings.h
 * @ingroup GameSettings
 * @brief TODO: Document this.
 */

#ifndef CARD_COUNTER_SETTINGS_H
#define CARD_COUNTER_SETTINGS_H

#include <libcockatrice/settings/settings_manager.h>

class QSettings;
class QColor;

class CardCounterSettings : public SettingsManager
{
    Q_OBJECT

public:
    CardCounterSettings(const QString &settingsPath, QObject *parent = nullptr);

    [[nodiscard]] QColor color(int counterId) const;

    [[nodiscard]] QString displayName(int counterId) const;

public slots:
    void setColor(int counterId, const QColor &color);

signals:
    void colorChanged(int counterId, const QColor &color);
};

#endif // CARD_COUNTER_SETTINGS_H
