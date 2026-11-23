/**
 * @file deck_analysis_url.h
 * @ingroup ApiInterfaces
 */

#ifndef DECK_ANALYSIS_URL_H
#define DECK_ANALYSIS_URL_H

#include <QString>
#include <QUrl>

/**
 * Validates a deck address returned by a deck analysis service.
 *
 * Only the expected host (optionally "www."-prefixed), HTTPS, default ports and
 * URLs without user information are accepted. Legacy HTTP addresses for the same
 * service are upgraded to HTTPS in place instead of being rejected.
 *
 * Relative addresses must be resolved against the response URL by the caller.
 *
 * @return true when @p url names a deck on @p serviceHost; false otherwise.
 */
inline bool normalizeAnalysisUrl(QUrl &url, const QString &serviceHost)
{
    const QString host = url.host().toLower();
    const bool legacyHttp = url.scheme() == QStringLiteral("http");
    const int defaultPort = legacyHttp ? 80 : 443;
    if (!url.isValid() || (!legacyHttp && url.scheme() != QStringLiteral("https")) || !url.userInfo().isEmpty() ||
        !url.path().startsWith('/') || (url.port() != -1 && url.port() != defaultPort) ||
        (host != serviceHost && host != QStringLiteral("www.") + serviceHost)) {
        return false;
    }
    // Both services accept HTTPS. Older public metadata may still identify the
    // same deck with HTTP; retain its host/path/query while using HTTPS to open it.
    if (legacyHttp) {
        url.setScheme(QStringLiteral("https"));
        url.setPort(-1);
    }
    return true;
}

#endif
