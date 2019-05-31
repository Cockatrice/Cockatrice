/*
 * Beware of this preprocessor hack used to redefine the settingCache class
 * instead of including it and all of its dependencies.
 */

#include <QObject>
#include <QString>

#define SETTINGSCACHE_H
#define PICTURELOADER_H

#include "../../cockatrice/src/carddatabase.h"

class CardDatabaseSettings
{
public:
    void setSortKey(QString shortName, unsigned int sortKey);
    void setEnabled(QString shortName, bool enabled);
    void setIsKnown(QString shortName, bool isknown);

    unsigned int getSortKey(QString shortName);
    bool isEnabled(QString shortName);
    bool isKnown(QString shortName);
};

class SettingsCache : public QObject
{
    Q_OBJECT
private:
    CardDatabaseSettings *cardDatabaseSettings;

public:
    SettingsCache();
    ~SettingsCache();
    QString getCustomCardDatabasePath() const;
    QString getCardDatabasePath() const;
    QString getTokenDatabasePath() const;
    QString getSpoilerCardDatabasePath() const;
    CardDatabaseSettings &cardDatabase() const;
signals:
    void cardDatabasePathChanged();
};

extern SettingsCache *settingsCache;

class PictureLoader
{
public:
    static void clearPixmapCache(CardInfoPtr card);
};
