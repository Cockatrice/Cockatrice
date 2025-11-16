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
    explicit SettingsManager(const QString &settingPath,
                             const QString &defaultGroup = QString(),
                             const QString &defaultSubGroup = QString(),
                             QObject *parent = nullptr);
    QVariant getValue(const QString &name);
    QVariant getValue(const QString &name, const QString &group, const QString &subGroup = QString());
    void sync();

protected:
    QSettings settings;
    QString defaultGroup;
    QString defaultSubGroup;
    void setValue(const QVariant &value, const QString &name);
    void
    setValue(const QVariant &value, const QString &name, const QString &group, const QString &subGroup = QString());
    void deleteValue(const QString &name);
    void deleteValue(const QString &name, const QString &group, const QString &subGroup = QString());
};

#endif // SETTINGSMANAGER_H
