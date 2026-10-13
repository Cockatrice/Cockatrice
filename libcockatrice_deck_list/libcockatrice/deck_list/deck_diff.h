/**
 * @file deck_diff.h
 * @brief Card-level diff and three-way merge for deck collection sync.
 */

#ifndef DECK_DIFF_H
#define DECK_DIFF_H

#include <QList>
#include <QString>

class DeckList;

/** @brief A single card-count change within one zone. */
struct DeckCardChange
{
    QString zone;     ///< Zone name, e.g. "main".
    QString cardName; ///< Card name.
    int oldCount = 0; ///< Copies in the old deck.
    int newCount = 0; ///< Copies in the new deck.
    QString cardSetShortName; ///< Printing set carried from the new side, empty when unchanged.
    QString cardSetNumber;    ///< Printing collector number carried from the new side.
    QString cardProviderId;   ///< Printing provider id carried from the new side.
    /** @brief Copies added (positive) or removed (negative). */
    [[nodiscard]] int delta() const
    {
        return newCount - oldCount;
    }
};

/** @brief Card-level difference between two decks, sorted by zone then card name. */
struct DeckDiffResult
{
    QList<DeckCardChange> changes;
    /** @brief True when both decks hold the same cards in the same zones. */
    [[nodiscard]] bool isEmpty() const
    {
        return changes.isEmpty();
    }
};

/**
 * @class DeckDiff
 * @brief Pure card-count diff and three-way merge for deck sync.
 *
 * Compares card names and copy counts per zone only; a merged deck keeps
 * base printings for existing cards and carries the changed side's printing
 * for newly added cards.
 */
class DeckDiff
{
public:
    /** @brief Diffs oldDeck against newDeck. */
    static DeckDiffResult diff(const DeckList &oldDeck, const DeckList &newDeck);
    /**
     * @brief True when both change sets touch disjoint (zone, card) keys and a merge needs no arbitration.
     */
    static bool canAutoMerge(const DeckDiffResult &localChanges, const DeckDiffResult &remoteChanges);
    /**
     * @brief Applies both change sets onto a copy of baseDeck.
     *
     * @precondition localChanges and remoteChanges were both computed with
     * diff() against baseDeck. Stale or foreign change sets silently
     * resurrect or delete cards.
     *
     * Counts are clamped at zero. On overlapping keys the remote change wins;
     * callers offer this result as "Use Merge" only when canAutoMerge is true.
     * Metadata and sideboard plans come from baseDeck.
     */
    static DeckList
    merge(const DeckList &baseDeck, const DeckDiffResult &localChanges, const DeckDiffResult &remoteChanges);
};

#endif // DECK_DIFF_H
