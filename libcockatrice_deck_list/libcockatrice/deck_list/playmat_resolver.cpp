#include "playmat_resolver.h"

#include <QRandomGenerator>

PlaymatInfo resolveEffectivePlaymat(const DeckList &deck,
                                    const PlaymatInfo &force,
                                    const QList<PlaymatInfo> &fallbackList,
                                    PlaymatFallbackMode fallbackMode,
                                    int rotationIndex)
{
    if (!force.card.isEmpty()) {
        return force;
    }

    const PlaymatInfo &deckPlaymat = deck.getPlaymat();
    if (!deckPlaymat.card.isEmpty()) {
        return deckPlaymat;
    }

    if (fallbackList.isEmpty()) {
        return {};
    }

    switch (fallbackMode) {
        case PlaymatFallbackModeFixed:
            return fallbackList.first();
        case PlaymatFallbackModeRoundRobin:
            return fallbackList.at(rotationIndex % fallbackList.size());
        case PlaymatFallbackModeRandom:
            return fallbackList.at(QRandomGenerator::global()->bounded(fallbackList.size()));
    }

    return {};
}

PlaymatInfo resolvePlaymatForDeck(const DeckList &deck,
                                  const QList<PlaymatInfo> &fallbackList,
                                  PlaymatMode mode,
                                  PlaymatFallbackMode fallbackBehavior,
                                  int rotationIndex)
{
    switch (mode) {
        case PlaymatModeOverrideDeck: {
            const DeckList emptyDeck;
            return resolveEffectivePlaymat(emptyDeck, {}, fallbackList, fallbackBehavior, rotationIndex);
        }
        case PlaymatModeFallback:
            return resolveEffectivePlaymat(deck, {}, fallbackList, fallbackBehavior, rotationIndex);
        case PlaymatModeDeckOnly:
            return deck.getPlaymat();
    }

    return {};
}
