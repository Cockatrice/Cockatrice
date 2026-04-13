#ifndef COCKATRICE_CARD_PICTURE_LOADER_CACHE_METHOD_H
#define COCKATRICE_CARD_PICTURE_LOADER_CACHE_METHOD_H
#include <QList>
#include <QString>

namespace CardPictureLoaderCacheMethod
{
enum class CacheMethod
{
    NETWORK_CACHE,
    FILESYSTEM_CACHE
};

struct CacheMethodInfo
{
    CacheMethod id;
    QString displayName;
};

static inline const QList<CacheMethodInfo> methods()
{
    static QList<CacheMethodInfo> all = {
        {CacheMethod::NETWORK_CACHE, "Network Cache"},
        {CacheMethod::FILESYSTEM_CACHE, "Filesystem Cache"},
    };
    return all;
}
} // namespace CardPictureLoaderCacheMethod

#endif // COCKATRICE_CARD_PICTURE_LOADER_CACHE_METHOD_H
