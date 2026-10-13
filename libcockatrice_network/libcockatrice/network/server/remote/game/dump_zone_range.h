#ifndef DUMP_ZONE_RANGE_H
#define DUMP_ZONE_RANGE_H

#include <QtGlobal>
#include <utility>

/**
 * @brief Resolve a client's zone dump request against a zone's real card count.
 *
 * @c number_cards arrives from the client unvalidated. -1 means "the whole zone", which
 * is the convention the client uses when sizing its own view of one. Every other negative
 * value, and every value larger than the zone, has to be brought into range rather than
 * used as an index: with a reversed request the zone offset is
 * @c cardCount - @c numberCards, which runs off the front of the list when the request
 * exceeds the zone and off the back when @c numberCards is -1.
 *
 * Kept in its own translation unit so it can be exercised without the rest of
 * server_abstract_player.cpp, which refers to the RNG global that only the servatrice
 * binary defines.
 *
 * @param cardCount number of cards the zone actually holds
 * @param numberCards the requested count, -1 for the whole zone
 * @param isReversed take the trailing cards instead of the leading ones
 * @return the half-open index range [begin, end) into the zone's card list, always
 *         contained in [0, cardCount)
 */
std::pair<qsizetype, qsizetype> dumpZoneCardRange(qsizetype cardCount, int numberCards, bool isReversed);

#endif // DUMP_ZONE_RANGE_H
