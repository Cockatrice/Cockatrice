#include "card_list.h"

#include "../cards/card_info.h"
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
 * Creates a String for the card such that when sorting cards using that string, it will result in the
 * following sort order:
 * - Unrecognized colors
 * - Land cards
 * - Colorless cards
 * - Monocolor cards, in wubrg order
 * - Monocolor cards of any custom colors
 * - 2C cards (no internal order)
 * - 3C cards (no internal order)
 * - 4C cards (no internal order)
 * - 5C cards (no internal order)
 *
 * @param c The card info
 * @param appendAtEnd For multicolor cards, whether to also append the entire color string at the end.
 */
static QString getColorSortString(const CardInfo &c, bool appendAtEnd)
{
    QString colors = c.getColors();
    switch (colors.size()) {
        case 0: {
            if (c.getCardType().contains("Land")) {
                return "a_land";
            } else {
                return "b_colorless";
            }
        }
        case 1:
            // force wubrg order
            switch (colors.at(0).toLatin1()) {
                case 'W':
                    return "c_W";
                case 'U':
                    return "d_U";
                case 'B':
                    return "e_B";
                case 'R':
                    return "f_R";
                case 'G':
                    return "g_G";
                default:
                    // handle any custom colors
                    return QString("h_%1").arg(colors.at(0));
            }
        default:
            return QString("i%1_%2").arg(colors.size()).arg(appendAtEnd ? colors : "");
    }
}

/**
 * @brief returns the function that extracts the given property from the CardItem.
 */
std::function<QString(CardItem *)> CardList::getExtractorFor(SortOption option)
{
    switch (option) {
        case NoSort:
            return [](CardItem *) { return ""; };
        case SortByMainType:
            return [](CardItem *c) { return c->getCardInfo().getMainCardType(); };
        case SortByManaValue:
            // getCmc returns the int as a string. We pad with 0's so that string comp also works on it
            return [](CardItem *c) { return c->getCard() ? c->getCardInfo().getCmc().rightJustified(4, '0') : ""; };
        case SortByColorGrouping:
            return [](CardItem *c) { return c->getCard() ? getColorSortString(c->getCardInfo(), false) : ""; };
        case SortByName:
            return [](CardItem *c) { return c->getName(); };
        case SortByType:
            return [](CardItem *c) { return c->getCardInfo().getCardType(); };
        case SortByManaCost:
            return [](CardItem *c) {
                if (!c->getCard()) {
                    return QString();
                }

                auto info = c->getCardInfo();

                // calculation copied from CardDatabaseModel.
                // we pad the cmc and also append the mana cost to the end so same cmc cards still have a sort order
                return QString("%1%2").arg(info.getCmc(), 4, QChar('0')).arg(info.getManaCost());
            };
        case SortByColors:
            return [](CardItem *c) { return c->getCard() ? getColorSortString(c->getCardInfo(), true) : ""; };
        case SortByPt:
            // do the same padding trick as above
            return
                [](CardItem *c) { return c->getCard() ? c->getCardInfo().getPowTough().rightJustified(10, '0') : ""; };
        case SortBySet:
            return [](CardItem *c) { return c->getCardInfo().getSetsNames(); };
        case SortByPrinting:
            return [](CardItem *c) { return c->getProviderId(); };
    }

    // this line should never be reached
    qCWarning(CardListLog) << "cardlist.cpp: Could not find extractor for SortOption" << option;
    return [](CardItem *) { return ""; };
}