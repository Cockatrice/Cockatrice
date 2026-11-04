/**
 * @file settings_manager.h
 * @ingroup Settings
 */
//! \todo Document this file.

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <functional>
#include <qtmetamacros.h>

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
    QVariant
    getValue(const QString &name, const QString &group, const QString &subGroup, const QVariant &defaultValue) const;
    //! \brief Runs \a batchWriteFunction on a temporary QSettings and syncs once.
    //! \return the status of the underlying file after the final sync.
    QSettings::Status batchWrite(std::function<void(QSettings &)> batchWriteFunction);

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
