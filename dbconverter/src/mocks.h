/*
 * Beware of this preprocessor hack used to redefine the settingCache class
 * instead of including it and all of its dependencies.
 */

#include <QObject>
#include <QString>

#define PICTURELOADER_H

#include "../../cockatrice/src/carddatabase.h"
#include "../../cockatrice/src/settingscache.h"

extern SettingsCache *settingsCache;

class PictureLoader
{
public:
    static void clearPixmapCache(CardInfoPtr card);
};
