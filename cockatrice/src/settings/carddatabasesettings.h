#ifndef CARDDATABASESETTINGS_H
#define CARDDATABASESETTINGS_H

#include "settingsmanager.h"

#include <QObject>
#include <QVariant>
#include <QSettings>

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
    CardDatabaseSettings(QString settingPath, QObject *parent = 0);
    CardDatabaseSettings( const CardDatabaseSettings& /*other*/ );
    CardDatabaseSettings( CardDatabaseSettings& /*other*/ );
    CardDatabaseSettings( volatile const CardDatabaseSettings& /*other*/ );
    CardDatabaseSettings( volatile CardDatabaseSettings& /*other*/ );    
};

#endif // CARDDATABASESETTINGS_H
