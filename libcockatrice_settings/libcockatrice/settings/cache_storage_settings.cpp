#include "cache_storage_settings.h"

CacheStorageSettings::CacheStorageSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "cache_storage.ini", "cache_storage", QString(), parent)
{
}

int CacheStorageSettings::getPixmapCacheSize() const
{
    return getValue("pixmapCacheSize", QString(), QString(), PIXMAPCACHE_SIZE_DEFAULT).toInt();
}

int CacheStorageSettings::getNetworkCacheSizeInMB() const
{
    return getValue("networkCacheSize", QString(), QString(), NETWORK_CACHE_SIZE_DEFAULT).toInt();
}

int CacheStorageSettings::getRedirectCacheTtl() const
{
    return getValue("redirectCacheTtl", QString(), QString(), NETWORK_REDIRECT_CACHE_TTL_DEFAULT).toInt();
}

int CacheStorageSettings::getCardPictureLoaderCacheMethod() const
{
    return getValue("cardPictureLoaderCacheMethod", QString(), QString(), 0).toInt();
}

int CacheStorageSettings::getLocalCardImageStorageNamingScheme() const
{
    return getValue("localCardImageStorageNamingScheme", QString(), QString(), LOCAL_CARD_IMAGE_NAMING_SCHEME_DEFAULT)
        .toInt();
}

void CacheStorageSettings::setPixmapCacheSize(int _pixmapCacheSize)
{
    setValue(_pixmapCacheSize, "pixmapCacheSize");
    emit pixmapCacheSizeChanged(_pixmapCacheSize);
}

void CacheStorageSettings::setNetworkCacheSizeInMB(int _networkCacheSize)
{
    setValue(_networkCacheSize, "networkCacheSize");
    emit networkCacheSizeChanged(_networkCacheSize);
}

void CacheStorageSettings::setNetworkRedirectCacheTtl(int _redirectCacheTtl)
{
    setValue(_redirectCacheTtl, "redirectCacheTtl");
    emit redirectCacheTtlChanged(_redirectCacheTtl);
}

void CacheStorageSettings::setCardImageCacheMethod(int _cardImageCachingMethod)
{
    setValue(_cardImageCachingMethod, "cardPictureLoaderCacheMethod");
    emit cardPictureLoaderCacheMethodChanged(_cardImageCachingMethod);
}

void CacheStorageSettings::setLocalCardImageStorageNamingScheme(int _localCardImageStorageNamingScheme)
{
    setValue(_localCardImageStorageNamingScheme, "localCardImageStorageNamingScheme");
    emit localCardImageStorageNamingSchemeChanged(_localCardImageStorageNamingScheme);
}
