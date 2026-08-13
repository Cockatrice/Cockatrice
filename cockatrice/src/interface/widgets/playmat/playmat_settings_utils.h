#ifndef COCKATRICE_PLAYMAT_SETTINGS_UTILS_H
#define COCKATRICE_PLAYMAT_SETTINGS_UTILS_H

#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/playmat_resolver.h>
#include <libcockatrice/interfaces/interface_interface_settings_provider.h>

class InterfaceSettings;

/**
 * @brief Converts a stored (settings-level) playmat entry into resolver types,
 *        clamping the positioning parameters to their valid ranges.
 */
PlaymatResolution resolutionFromStoredPlaymat(const StoredPlaymat &stored);

/**
 * @brief Converts a resolved playmat into a stored (settings-level) entry.
 */
StoredPlaymat storedFromResolution(const PlaymatResolution &resolution);

/**
 * @brief Applies the user-level playmat settings (override, fallback list and
 *        mode) to a copy of the deck, returning the deck with the effective
 *        playmat baked into its metadata.
 *
 * The resolution chain is: override > deck-configured playmat > fallback list.
 * The deck passed in is never modified; the returned copy is intended for
 * upload to the server so local and remote players see the same effective mat.
 */
DeckList applyUserPlaymatSettings(const DeckList &deck, const InterfaceSettings &settings);

#endif // COCKATRICE_PLAYMAT_SETTINGS_UTILS_H
