/**
 * @file card_counter_settings.h
 * @ingroup Settings
 * @brief TODO: Document this.
 */

#ifndef CARD_COUNTER_SETTINGS_H
#define CARD_COUNTER_SETTINGS_H

#include "settings_manager.h"

#include <QObject>

class QSettings;
class QColor;

class CardCounterSettings : public SettingsManager
{
    Q_OBJECT

public:
    CardCounterSettings(const QString &settingsPath, QObject *parent = nullptr);

    QColor color(int counterId) const;

    QString displayName(int counterId) const;

public slots:
    void setColor(int counterId, const QColor &color);

signals:
    void colorChanged(int counterId, const QColor &color);
};

#endif // CARD_COUNTER_SETTINGS_H
