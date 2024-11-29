#include "card_list.h"

#include "card_database.h"
#include "card_item.h"

#include <QDebug>
#include <algorithm>

CardList::CardList(bool _contentsKnown) : QList<CardItem *>(), contentsKnown(_contentsKnown)
{
}

/**
 * @brief Finds the CardItem with the given id in the list.
 * If contentsKnown is false, then this just returns the first element of the list.
 *
 * @param cardId The id of the card to find.
 *
 * @returns A pointer to the CardItem, or a nullptr if not found.
 */
CardItem *CardList::findCard(const int cardId) const
{
    if (!contentsKnown && !empty()) {
        return at(0);
    } else {
        for (auto *cardItem : *this) {
            if (cardItem->getId() == cardId) {
                return cardItem;
            }
        }
    }
    return nullptr;
}

/**
 * @brief sorts the list by using string comparison on properties extracted from the CardItem
 * The cards are compared using each property in order.
 * If two cards have the same value for a property, then the next property in the list is used.
 *
 * @param option the option to compare the cards by, in order of usage.
 */
void CardList::sortBy(const QList<SortOption> &option)
{
    // early return if we know we won't be sorting
    if (option.isEmpty()) {
        return;
    }

    auto comparator = [&option](CardItem *a, CardItem *b) {
        for (auto prop : option) {
            auto extractor = getExtractorFor(prop);
            QString t1 = extractor(a);
            QString t2 = extractor(b);
            if (t1 != t2) {
                return t1 < t2;
            }
        }
        return false;
    };

    std::sort(begin(), end(), comparator);
}

/**
 * @brief returns the function that extracts the given property from the CardItem.
 */
std::function<QString(CardItem *)> CardList::getExtractorFor(SortOption option)
{
    switch (option) {
        case NoSort:
            return [](CardItem *) { return ""; };
        case SortByName:
            return [](CardItem *c) { return c->getName(); };
        case SortByType:
            return [](CardItem *c) { return c->getInfo() ? c->getInfo()->getMainCardType() : ""; };
        case SortByManaValue:
            // getCmc returns the int as a string. We pad with 0's so that string comp also works on it
            return [](CardItem *c) { return c->getInfo() ? c->getInfo()->getCmc().rightJustified(4, '0') : ""; };
    }

    // this line should never be reached
    qDebug() << "cardlist.cpp: Could not find extractor for SortOption" << option;
    return [](CardItem *) { return ""; };
}