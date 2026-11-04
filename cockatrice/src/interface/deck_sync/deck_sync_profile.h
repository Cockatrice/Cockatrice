/**
 * @file deck_sync_profile.h
 * @brief Identity of one sync target (server + user) for deck collection sync.
 */

#ifndef DECK_SYNC_PROFILE_H
#define DECK_SYNC_PROFILE_H

#include <QString>

/**
 * @class DeckSyncProfile
 * @brief Identifies which server and login a sync sidecar belongs to.
 *
 * A local deck folder can sync against several servers. Every server login
 * gets its own profile, and every profile gets its own sidecar file, so
 * switching servers never mixes up remote deck ids.
 */
class DeckSyncProfile
{
public:
    QString host;     ///< Server hostname as entered by the user.
    int port = 0;     ///< Server port.
    QString username; ///< Login name on that server.

    DeckSyncProfile() = default;
    DeckSyncProfile(const QString &_host, int _port, const QString &_username);

    /** @brief False when any of host, port or username is missing. */
    [[nodiscard]] bool isValid() const;
    /**
     * @brief Stable file-system-safe key for this profile.
     *
     * SHA1 hex over the NUL-separated lowercased "host", port and "username"
     * triple, so the same server reached with different hostname
     * capitalisation maps to one file, while IPv6 hosts and usernames
     * containing ':' can never collide two profiles. Empty for invalid
     * profiles, so no two half-configured profiles share a file.
     */
    [[nodiscard]] QString key() const;
};

#endif // DECK_SYNC_PROFILE_H
