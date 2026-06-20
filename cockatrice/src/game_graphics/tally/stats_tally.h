#ifndef COCKATRICE_POWER_TALLY_H
#define COCKATRICE_POWER_TALLY_H
#include "tally.h"

/**
 * @brief Extracts and tallies stats from selected cards.
 */
namespace StatsTally
{

/**
 * @brief Sums the power of all selected cards
 *
 * @param cards The list of selected card items to analyze.
 * @return A single row containing the total, or an empty list if none of the cards have pt
 */
QList<TallyRow> computeTotalPower(const QList<CardItem *> &cards);

} // namespace StatsTally

#endif // COCKATRICE_POWER_TALLY_H
