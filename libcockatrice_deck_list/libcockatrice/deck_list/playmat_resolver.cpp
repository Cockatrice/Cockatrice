#include "playmat_resolver.h"

#include <QRandomGenerator>

PlaymatResolution resolveEffectivePlaymat(const DeckList &deck,
                                          const PlaymatResolution &force,
                                          const QList<PlaymatResolution> &fallbackList,
                                          PlaymatFallbackMode fallbackMode,
                                          int &rotationIndex)
{
    if (!force.card.isEmpty()) {
        return force;
    }

    if (!deck.getPlaymatCard().isEmpty()) {
        return {deck.getPlaymatCard(), deck.getPlaymatParams()};
    }

    if (fallbackList.isEmpty()) {
        return {};
    }

    switch (fallbackMode) {
        case PlaymatFallbackMode::Fixed:
            return fallbackList.first();
        case PlaymatFallbackMode::RoundRobin:
            return fallbackList.at(rotationIndex++ % fallbackList.size());
        case PlaymatFallbackMode::Random:
            return fallbackList.at(QRandomGenerator::global()->bounded(fallbackList.size()));
    }

    return {};
}
