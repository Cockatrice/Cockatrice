#ifndef COCKATRICE_PRINTING_REF_H
#define COCKATRICE_PRINTING_REF_H

#include <QString>

/**
 * @brief The fields used in a decklist card entry to pinpoint a printing of a card.
 */
struct PrintingRef
{
    QString setCode = "";         ///< Short set code (e.g., "NEO").
    QString collectorNumber = ""; ///< Collector number within the set.
    QString providerId = "";      ///< External provider identifier (e.g., UUID). Used in CardRef.
};

#endif // COCKATRICE_PRINTING_REF_H
