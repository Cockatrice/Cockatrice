#ifndef COCKATRICE_CARD_RELATION_TYPE_H
#define COCKATRICE_CARD_RELATION_TYPE_H

#include <QString>

/**
 * @enum CardRelationType
 * @ingroup Cards
 * @brief Types of attachments between cards.
 *
 * DoesNotAttach: No attachment is present.
 * AttachTo: This card attaches to another card.
 * TransformInto: This card transforms into another card.
 */
enum class CardRelationType
{
    DoesNotAttach = 0,
    AttachTo = 1,
    TransformInto = 2,
};

// Helper function to transform the enum values into human-readable strings
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
