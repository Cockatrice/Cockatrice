#include "tally.h"

#include "subtype_tally.h"

QList<TallyRow> Tally::compute(const QList<CardItem *> &cards, const TallyType type)
{
    switch (type) {
        case TallyType::None:
            return {};
        case TallyType::Subtypes:
            return SubtypeTally::countSubtypes(cards);
    }
    return {};
}
