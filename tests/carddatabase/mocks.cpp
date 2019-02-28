
#include "mocks.h"

void CardDatabaseSettings::setSortKey(QString /* shortName */, unsigned int /* sortKey */){};
void CardDatabaseSettings::setEnabled(QString /* shortName */, bool /* enabled */){};
void CardDatabaseSettings::setIsKnown(QString /* shortName */, bool /* isknown */){};
unsigned int CardDatabaseSettings::getSortKey(QString /* shortName */)
{
    return 0;
};
bool CardDatabaseSettings::isEnabled(QString /* shortName */)
{
    return true;
};
bool CardDatabaseSettings::isKnown(QString /* shortName */)
{
    return true;
};

SettingsCache::SettingsCache()
{
    cardDatabaseSettings = new CardDatabaseSettings();
};
SettingsCache::~SettingsCache()
{
    delete cardDatabaseSettings;
};
QString SettingsCache::getCustomCardDatabasePath() const
{
    return QString("%1/customsets/").arg(CARDDB_DATADIR);
}
QString SettingsCache::getCardDatabasePath() const
{
    return QString("%1/cards.xml").arg(CARDDB_DATADIR);
}
QString SettingsCache::getTokenDatabasePath() const
{
    return QString("%1/tokens.xml").arg(CARDDB_DATADIR);
}
QString SettingsCache::getSpoilerCardDatabasePath() const
{
    return QString("%1/spoiler.xml").arg(CARDDB_DATADIR);
}
CardDatabaseSettings &SettingsCache::cardDatabase() const
{
    return *cardDatabaseSettings;
}


void PictureLoader::clearPixmapCache(CardInfoPtr /* card */)
{
}

SettingsCache *settingsCache;
