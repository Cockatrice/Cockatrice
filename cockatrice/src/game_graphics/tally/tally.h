#ifndef COCKATRICE_TALLY_H
#define COCKATRICE_TALLY_H
#include <QString>

class CardItem;

/** @brief A single row of the tally output. */
struct TallyRow
{
    QString name;  ///< The row name (displayed on the left)
    QString value; ///< Value for the row (displayed on the right)

    bool operator==(const TallyRow &) const = default;
};

/**
 * The tally type
 */
enum class TallyType
{
    None,
    Subtypes,
};

namespace Tally
{

/**
 * @brief Analyzes the selected cards according to the tally type and builds the resulting tally rows.
 * This forwards the cards to the code for that tally type.
 *
 * @param cards The list of selected card items to analyze.
 * @param type The type of tally to do
 * @return Rows sorted in top-to-bottom display order
 */
QList<TallyRow> compute(const QList<CardItem *> &cards, TallyType type);

} // namespace Tally

#endif // COCKATRICE_TALLY_H
