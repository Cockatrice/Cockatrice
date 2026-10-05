#include "dump_zone_range.h"

#include <algorithm>

std::pair<qsizetype, qsizetype> dumpZoneCardRange(qsizetype cardCount, int numberCards, bool isReversed)
{
    qsizetype count = 0;
    if (numberCards == -1) {
        count = cardCount;
    } else if (numberCards > 0) {
        count = std::min<qsizetype>(numberCards, cardCount);
    }
    count = std::max<qsizetype>(count, 0);

    const qsizetype begin = isReversed ? cardCount - count : 0;
    return { begin, begin + count };
}
