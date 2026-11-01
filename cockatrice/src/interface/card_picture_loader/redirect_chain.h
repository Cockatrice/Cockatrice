#ifndef COCKATRICE_REDIRECT_CHAIN_H
#define COCKATRICE_REDIRECT_CHAIN_H

#include <QDateTime>
#include <QHash>
#include <QPair>
#include <QSet>
#include <QUrl>
#include <utility>

// Resolve previously accepted redirects without recursion. Invalid chains are
// removed so subsequent requests can rediscover them from the source server.
inline QUrl resolveImageRedirects(const QUrl &url, QHash<QUrl, QPair<QUrl, QDateTime>> &cache, int maxHops)
{
    QUrl resolved = url;
    QSet<QUrl> visited;
    for (int depth = 0; depth < maxHops; ++depth) {
        if (visited.contains(resolved)) {
            break;
        }
        visited.insert(resolved);
        const auto it = cache.constFind(resolved);
        if (it == cache.cend() || it.value().first.isEmpty()) {
            return resolved;
        }
        resolved = it.value().first;
    }
    for (const QUrl &entry : std::as_const(visited)) {
        cache.remove(entry);
    }
    return url;
}

#endif
