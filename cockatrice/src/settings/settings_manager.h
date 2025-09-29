/**
 * @file settings_manager.h
 * @ingroup Settings
 * @brief TODO: Document this.
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVariant>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(const QString &settingPath, QObject *parent = nullptr);
    QVariant getValue(const QString &name, const QString &group = "", const QString &subGroup = "");
    void sync();

protected:
    QSettings settings;
    void setValue(const QVariant &value, const QString &name, const QString &group = "", const QString &subGroup = "");
    void deleteValue(const QString &name, const QString &group = "", const QString &subGroup = "");
};

#endif // SETTINGSMANAGER_H
