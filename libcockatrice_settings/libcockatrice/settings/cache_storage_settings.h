#ifndef CACHE_STORAGE_SETTINGS_H
#define CACHE_STORAGE_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_cache_storage_settings_provider.h>

// In MB (Increments of 64)
#define PIXMAPCACHE_SIZE_DEFAULT 2048
#define PIXMAPCACHE_SIZE_MIN 64
#define PIXMAPCACHE_SIZE_MAX 4096

// In MB
constexpr int NETWORK_CACHE_SIZE_DEFAULT = 1024 * 4; // 4 GB
constexpr int NETWORK_CACHE_SIZE_MIN = 1;            // 1 MB
constexpr int NETWORK_CACHE_SIZE_MAX = 1024 * 1024;  // 1 TB

// In Days
#define NETWORK_REDIRECT_CACHE_TTL_DEFAULT 30
#define NETWORK_REDIRECT_CACHE_TTL_MIN 1
#define NETWORK_REDIRECT_CACHE_TTL_MAX 90

class CacheStorageSettings : public SettingsManager, public ICacheStorageSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] int getPixmapCacheSize() const override;
    [[nodiscard]] int getNetworkCacheSizeInMB() const override;
    [[nodiscard]] int getRedirectCacheTtl() const override;
    [[nodiscard]] int getCardPictureLoaderCacheMethod() const;
    [[nodiscard]] int getLocalCardImageStorageNamingScheme() const;

    void setPixmapCacheSize(int _pixmapCacheSize);
    void setNetworkCacheSizeInMB(int _networkCacheSize);
    void setNetworkRedirectCacheTtl(int _redirectCacheTtl);
    void setCardImageCacheMethod(int _cardImageCachingMethod);
    void setLocalCardImageStorageNamingScheme(int _scheme);

signals:
    void pixmapCacheSizeChanged(int newSizeInMBs);
    void networkCacheSizeChanged(int newSizeInMBs);
    void redirectCacheTtlChanged(int newTtl);
    void cardPictureLoaderCacheMethodChanged(int cardPictureLoaderCacheMethod);
    void localCardImageStorageNamingSchemeChanged(int localCardImageStorageNamingScheme);

#ifdef SETTINGS_UNIT_TEST
public:
#else
private:
#endif
    explicit CacheStorageSettings(const QString &settingPath, QObject *parent = nullptr);
    CacheStorageSettings(const CacheStorageSettings & /*other*/);
};

#endif // CACHE_STORAGE_SETTINGS_H
