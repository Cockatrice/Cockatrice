#ifndef COCKATRICE_CARD_RELATION_TYPE_H
#define COCKATRICE_CARD_RELATION_TYPE_H

#include <QString>

/**
 * Represents how a card relates to another (attach, transform, etc.).
 */
enum class CardRelationType
{
    DoesNotAttach = 0,
    AttachTo = 1,
    TransformInto = 2,
};

// Optional helper
inline QString cardAttachTypeToString(CardRelationType type)
{
    switch (type) {
        case CardRelationType::AttachTo:
            return "attach";
        case CardRelationType::TransformInto:
            return "transform";
        default:
            return "";
    }
}

#endif // COCKATRICE_CARD_RELATION_TYPE_H
