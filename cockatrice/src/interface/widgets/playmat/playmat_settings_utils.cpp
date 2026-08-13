#include "playmat_settings_utils.h"

#include <libcockatrice/interfaces/interface_interface_settings_provider.h>
#include <libcockatrice/settings/interface_settings.h>

PlaymatResolution resolutionFromStoredPlaymat(const StoredPlaymat &stored)
{
    PlaymatResolution resolution;
    resolution.card = {stored.name, stored.providerId};
    resolution.params = {qBound(0.0, stored.marginPctL, 0.95), qBound(0.0, stored.marginPctR, 0.95),
                         qBound(0.0, stored.verticalOffset, 1.0), qBound(0.1, stored.zoom, 4.0)};
    return resolution;
}

StoredPlaymat storedFromResolution(const PlaymatResolution &resolution)
{
    StoredPlaymat stored;
    stored.name = resolution.card.name;
    stored.providerId = resolution.card.providerId;
    stored.marginPctL = resolution.params.marginPctL;
    stored.marginPctR = resolution.params.marginPctR;
    stored.verticalOffset = resolution.params.verticalOffset;
    stored.zoom = resolution.params.zoom;
    return stored;
}

DeckList applyUserPlaymatSettings(const DeckList &deck, const InterfaceSettings &settings)
{
    // Per-session cursor for the round-robin mode; advancing per resolution
    // (per deck upload, i.e. roughly per game) cycles deterministically.
    static int rotationIndex = 0;

    QList<PlaymatResolution> fallbackList;
    const QList<StoredPlaymat> storedList = settings.getPlaymatFallbackList();
    fallbackList.reserve(storedList.size());
    for (const StoredPlaymat &stored : storedList) {
        fallbackList.append(resolutionFromStoredPlaymat(stored));
    }

    const PlaymatResolution resolved =
        resolveEffectivePlaymat(deck, resolutionFromStoredPlaymat(settings.getPlaymatOverride()), fallbackList,
                                static_cast<PlaymatFallbackMode>(settings.getPlaymatFallbackMode()), rotationIndex);

    DeckList effectiveDeck = deck;
    effectiveDeck.setPlaymatCard(resolved.card);
    effectiveDeck.setPlaymatParams(resolved.params);
    return effectiveDeck;
}
