/*
 * Beware of this preprocessor hack used to redefine the settingCache class
 * instead of including it and all of its dependencies.
 * Always set header guards of mocked objects before including any headers
 * with mocked objects.
 */

#include <QObject>
#include <QString>

#define PICTURELOADER_H

#include "../../cockatrice/src/card/card_database.h"
#include "../../cockatrice/src/settings/cache_settings.h"
#include "../../cockatrice/src/utility/macros.h"

extern SettingsCache *settingsCache;

class PictureLoader
{
public:
    static void clearPixmapCache(CardInfoPtr card);
};
