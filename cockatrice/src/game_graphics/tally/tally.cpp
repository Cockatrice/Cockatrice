#include "tally.h"

#include "subtype_tally.h"

TallyType Tally::intToType(int value)
{
    // remember to change this whenever you add a new member
    if (value < static_cast<int>(TallyType::None) || value >= static_cast<int>(TallyType::Subtypes)) {
        return TallyType::None;
    }

    return static_cast<TallyType>(value);
}

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
