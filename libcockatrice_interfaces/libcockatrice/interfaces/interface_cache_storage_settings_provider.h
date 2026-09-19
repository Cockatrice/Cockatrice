#ifndef COCKATRICE_INTERFACE_CACHE_STORAGE_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_CACHE_STORAGE_SETTINGS_PROVIDER_H

class ICacheStorageSettingsProvider
{
public:
    virtual ~ICacheStorageSettingsProvider() = default;

    [[nodiscard]] virtual int getPixmapCacheSize() const = 0;
    [[nodiscard]] virtual int getNetworkCacheSizeInMB() const = 0;
    [[nodiscard]] virtual int getRedirectCacheTtl() const = 0;
};

#endif // COCKATRICE_INTERFACE_CACHE_STORAGE_SETTINGS_PROVIDER_H
