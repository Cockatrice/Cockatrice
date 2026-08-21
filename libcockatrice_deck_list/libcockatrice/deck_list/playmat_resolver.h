#ifndef COCKATRICE_PLAYMAT_RESOLVER_H
#define COCKATRICE_PLAYMAT_RESOLVER_H

#include "deck_list.h"

#include <libcockatrice/interfaces/interface_interface_settings_provider.h>

/**
 * @brief Resolves the effective playmat for a deck per the resolution chain:
 *        force override > deck-configured playmat > fallback list > none.
 *
 * @param deck The deck to resolve a playmat for.
 * @param force An optional user-level override; wins over everything. Pass an
 *              empty @ref PlaymatInfo::card to skip it.
 * @param fallbackList User-level fallback playmats, consulted only when the
 *                     deck has no configured playmat.
 * @param fallbackMode How @p fallbackList is consulted (ignored when empty).
 * @param rotationIndex In/out cursor for @c PlaymatFallbackModeRoundRobin;
 *                      advanced once per call. Unused for the other modes.
 * @return The effective playmat; an empty @ref PlaymatInfo::card when
 *         nothing in the chain resolves.
 */
PlaymatInfo resolveEffectivePlaymat(const DeckList &deck,
                                    const PlaymatInfo &force,
                                    const QList<PlaymatInfo> &fallbackList,
                                    PlaymatFallbackMode fallbackMode,
                                    int rotationIndex);

/**
 * @brief Resolves the playmat to display for a deck according to the user's
 *        collection mode (@ref PlaymatMode), combining the deck with the
 *        given fallback list.
 *
 * @param deck The deck to resolve a playmat for.
 * @param fallbackList User-level fallback playmats.
 * @param mode How the collection interacts with the deck-configured playmat.
 * @param fallbackBehavior How @p fallbackList is picked from.
 * @param rotationIndex Cursor for @c PlaymatFallbackModeRoundRobin.
 * @return The effective playmat; an empty @ref PlaymatInfo::card when nothing resolves.
 */
PlaymatInfo resolvePlaymatForDeck(const DeckList &deck,
                                  const QList<PlaymatInfo> &fallbackList,
                                  PlaymatMode mode,
                                  PlaymatFallbackMode fallbackBehavior,
                                  int rotationIndex);

#endif // COCKATRICE_PLAYMAT_RESOLVER_H
