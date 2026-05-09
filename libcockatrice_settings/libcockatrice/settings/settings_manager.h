/**
 * @file settings_manager.h
 * @ingroup Settings
 * @brief TODO: Document this.
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

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

    QVariant getValue(const QString &name) const;
    QVariant getValue(const QString &name, const QString &group, const QString &subGroup = QString()) const;
    void batchWrite(std::function<void(QSettings &)> batchWriteFunction);

    void sync();

protected:
    QString settingPath;
    QString defaultGroup;
    QString defaultSubGroup;

    QSettings getSettings() const;

    void setValue(const QVariant &value, const QString &name);

    void
    setValue(const QVariant &value, const QString &name, const QString &group, const QString &subGroup = QString());

    void deleteValue(const QString &name);

    void deleteValue(const QString &name, const QString &group, const QString &subGroup = QString());
};

#endif // SETTINGSMANAGER_H
