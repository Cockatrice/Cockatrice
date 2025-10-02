/**
 * @file card_database_settings.h
 * @ingroup CardDatabase
 * @ingroup CardSettings
 * @brief TODO: Document this.
 */

#ifndef CARDDATABASESETTINGS_H
#define CARDDATABASESETTINGS_H

#include "settings_manager.h"

#include <QObject>
#include <QSettings>
#include <QVariant>

class CardDatabaseSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    void setSortKey(QString shortName, unsigned int sortKey);
    void setEnabled(QString shortName, bool enabled);
    void setIsKnown(QString shortName, bool isknown);

    unsigned int getSortKey(QString shortName);
    bool isEnabled(QString shortName);
    bool isKnown(QString shortName);
signals:

public slots:

private:
    explicit CardDatabaseSettings(const QString &settingPath, QObject *parent = nullptr);
    CardDatabaseSettings(const CardDatabaseSettings & /*other*/);
};

#endif // CARDDATABASESETTINGS_H
