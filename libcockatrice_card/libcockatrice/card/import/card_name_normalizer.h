#ifndef COCKATRICE_CARD_NAME_NORMALIZER_H
#define COCKATRICE_CARD_NAME_NORMALIZER_H

#include <QString>

/**
 * Functor that normalizes the raw card name parsed during a plaintext deck import into the card name that Cockatrice
 * uses.
 */
struct CardNameNormalizer
{
    QString operator()(const QString &cardNameString) const;
};

#endif // COCKATRICE_CARD_NAME_NORMALIZER_H
