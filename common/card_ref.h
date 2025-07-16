#ifndef CARD_REF_H
#define CARD_REF_H

#include <QString>

/**
 * The information passed over the server that is required to identify the exact card to display.
 *
 * @param name The name of the card. Should not be empty, unless to indicate the lack of a card.
 * @param providerId Determines which printing of the card to use. Can be empty, in which case Cockatrice should default
 * to using the preferred set.
 */
struct CardRef
{
    QString name;
    QString providerId = QString();

    bool operator==(const CardRef &other) const
    {
        return name == other.name && providerId == other.providerId;
    }
};

#endif // CARD_REF_H
