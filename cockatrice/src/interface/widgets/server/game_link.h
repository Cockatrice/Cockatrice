/**
 * @file game_link.h
 * @ingroup UI
 * @brief Builds cockatrice://joingame links that let another user join a server game.
 */

#ifndef GAME_LINK_H
#define GAME_LINK_H

#include <QString>

/**
 * Builds a cockatrice://joingame link for the given server game. The receiver's
 * client opens it through the intent chain (connect -> join room -> join game).
 * @p description, when non-empty, is embedded in the link as the URL-encoded
 * "game" query item so the receiving client can name the game in its confirm
 * prompt and chat anchor instead of only its numeric id. Links built without it
 * stay valid: the parser and chat renderer fall back to the id alone.
 */
QString
makeGameJoinLink(const QString &hostname, int port, int roomId, int gameId, const QString &description = QString());

/**
 * One game the inviter is currently in and can invite another user to.
 * @p label is meant for display in menus, @p url is the ready-made invite link.
 * @p description is the raw game description for building tr()-wrapped invite
 * messages (the label already embeds it, but the send sites need the raw value).
 * @p onlyBuddies and @p creatorName mirror the server game's room settings so
 * callers can gate the invite to the creator's buddies.
 */
struct GameInviteOption
{
    int gameId = 0;
    QString label;
    QString url;
    QString description;
    bool onlyBuddies = false;
    QString creatorName;
};

#endif // GAME_LINK_H
