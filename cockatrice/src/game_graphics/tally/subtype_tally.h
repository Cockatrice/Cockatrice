#ifndef COCKATRICE_SUBTYPE_TALLY_H
#define COCKATRICE_SUBTYPE_TALLY_H

#include "tally.h"

#include <QList>
#include <QString>

class CardItem;

/**
 * @brief Extracts and tallies subtypes from selected cards.
 */
namespace SubtypeTally
{
/**
 * @brief Parses card type lines and counts each subtype occurrence.
 *
 * Skips face-down cards and cards without type info.
 * @param cards The list of selected card items to analyze.
 * @return Entries sorted by count ascending, then alphabetically.
 */
QList<TallyRow> countSubtypes(const QList<CardItem *> &cards);
} // namespace SubtypeTally

#endif
