#ifndef COCKATRICE_PLAYMAT_PARAMS_H
#define COCKATRICE_PLAYMAT_PARAMS_H

#include "card_ref.h"

#include <cmath>

/**
 * @struct PlaymatParams
 * @ingroup Decks
 * @brief Positioning parameters for a playmat card image.
 *
 * Controls how the cropped card art is positioned within the
 * combined table+stack play area. The coordinate system is
 * relative to the cropped art source image.
 */
struct PlaymatParams
{
    double marginPctL = 0.07;     ///< Left margin as fraction of card width (0.0–0.95).
    double marginPctR = 0.07;     ///< Right margin as fraction of card width (0.0–0.95).
    double verticalOffset = 0.33; ///< Vertical position within card (0.0=top, 1.0=bottom).
    double zoom = 1.0;            ///< Scale factor (0.1–4.0).

    bool operator==(const PlaymatParams &other) const
    {
        return qFuzzyCompare(marginPctL, other.marginPctL) && qFuzzyCompare(marginPctR, other.marginPctR) &&
               qFuzzyCompare(verticalOffset, other.verticalOffset) && qFuzzyCompare(zoom, other.zoom);
    }

    bool operator!=(const PlaymatParams &other) const
    {
        return !(*this == other);
    }
};

/**
 * @struct PlaymatResolution
 * @ingroup Decks
 * @brief A resolved playmat (card + positioning parameters).
 */
struct PlaymatResolution
{
    CardRef card;         ///< The card whose art is used as playmat.
    PlaymatParams params; ///< Positioning parameters for the playmat card image.

    bool operator==(const PlaymatResolution &other) const
    {
        return card == other.card && params == other.params;
    }
};

#endif // COCKATRICE_PLAYMAT_PARAMS_H
