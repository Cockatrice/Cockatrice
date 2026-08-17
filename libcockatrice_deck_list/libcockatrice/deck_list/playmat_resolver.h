#ifndef COCKATRICE_PLAYMAT_RESOLVER_H
#define COCKATRICE_PLAYMAT_RESOLVER_H

#include "deck_list.h"

/**
 * @brief How the user-level fallback playmat list is consulted when a deck has
 *        no playmat configured.
 */
enum class PlaymatFallbackMode
{
    Fixed = 0,      ///< Always use the first entry of the fallback list.
    RoundRobin = 1, ///< Cycle through the list, advancing one entry per resolution.
    Random = 2      ///< Pick a random entry per resolution.
};

/**
 * @brief Resolves the effective playmat for a deck per the resolution chain:
 *        force override > deck-configured playmat > fallback list > none.
 *
 * @param deck The deck to resolve a playmat for.
 * @param force An optional user-level override; wins over everything. Pass an
 *              empty @ref PlaymatResolution::card to skip it.
 * @param fallbackList User-level fallback playmats, consulted only when the
 *                     deck has no configured playmat.
 * @param fallbackMode How @p fallbackList is consulted (ignored when empty).
 * @param rotationIndex In/out cursor for @c PlaymatFallbackMode::RoundRobin;
 *                      advanced once per call. Unused for the other modes.
 * @return The effective playmat; an empty @ref PlaymatResolution::card when
 *         nothing in the chain resolves.
 */
PlaymatResolution resolveEffectivePlaymat(const DeckList &deck,
                                          const PlaymatResolution &force,
                                          const QList<PlaymatResolution> &fallbackList,
                                          PlaymatFallbackMode fallbackMode,
                                          int rotationIndex);

#endif // COCKATRICE_PLAYMAT_RESOLVER_H
