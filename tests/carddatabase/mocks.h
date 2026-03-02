/*
 * Beware of this preprocessor hack used to redefine the settingCache class
 * instead of including it and all of its dependencies.
 * Always set header guards of mocked objects before including any headers
 * with mocked objects.
 */

#define PICTURELOADER_H

#include <libcockatrice/card/database/card_database.h>

class CardPictureLoader
{
public:
    static void clearPixmapCache(CardInfoPtr card);
};
