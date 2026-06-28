#ifndef SELECTION_SUBTYPE_TALLY_H
#define SELECTION_SUBTYPE_TALLY_H

#include <QList>
#include <QString>

class CardItem;

/** @brief A single subtype (e.g., "Goblin", "Warrior") with its occurrence count. */
struct SubtypeEntry
{
    QString name; ///< The subtype name
    int count;    ///< Number of selected cards with this subtype

    bool operator==(const SubtypeEntry &other) const
    {
        return name == other.name && count == other.count;
    }
};

/**
 * @brief Extracts and tallies subtypes from selected cards.
 */
namespace SelectionSubtypeTally
{
/**
 * @brief Parses card type lines and counts each subtype occurrence.
 *
 * Skips face-down cards and cards without type info.
 * @param cards The list of selected card items to analyze.
 * @return Entries sorted by count ascending, then alphabetically.
 */
QList<SubtypeEntry> countSubtypes(const QList<CardItem *> &cards);
} // namespace SelectionSubtypeTally

#endif
