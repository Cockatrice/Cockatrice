#ifndef COCKATRICE_CARD_RELATION_H
#define COCKATRICE_CARD_RELATION_H

#include "card_relation_type.h"

#include <QString>
#include <qtmetamacros.h>

/**
 * @struct CardRelation
 * @ingroup Cards
 *
 * @brief Represents a relationship between two cards.
 *
 * CardRelation objects define directional relationships, such as:
 * - One card attaching to another.
 * - One card transforming into another.
 * - One card creating another instance.
 *
 * Relations may also define metadata such as whether multiple creations
 * are possible, whether the relation is persistent, and default counts.
 */
struct CardRelation
{
    QString name;                                                  ///< Name of the related card.
    CardRelationType attachType = CardRelationType::DoesNotAttach; ///< Type of attachment.
    bool isCreateAllExclusion = false; ///< True if this relation should be excluded from "create all" operation.
    bool isVariableCount = false;      ///< True if the number of creations is variable.
    int defaultCount = 1;              ///< Default number of cards created or involved.
    bool isPersistent = false;         ///< True if this relation persists (i.e. is not destroyed) on zone change.
    bool isFaceDown = false;           ///< True if this relation creates the tokens facedown

    /**
     * @brief Returns true if the two relations are equal.
     * Two relations are equal if all their fields are equal.
     *
     * @return True if the two relations are equal
     */
    bool operator==(const CardRelation &) const = default;

    /**
     * @brief Returns true if the card is attached to another.
     *
     * @return True if attached, false otherwise.
     */
    [[nodiscard]] bool doesAttach() const
    {
        return attachType != CardRelationType::DoesNotAttach;
    }

    /**
     * @brief Returns true if this card transforms into another card.
     *
     * @return True if it transforms, false otherwise.
     */
    [[nodiscard]] bool doesTransform() const
    {
        return attachType == CardRelationType::TransformInto;
    }

    /**
     * @brief Returns a string description of the attachment type.
     *
     * @return "attach" for AttachTo, "transform" for TransformInto, empty string otherwise.
     */
    [[nodiscard]] QString getAttachTypeAsString() const
    {
        return cardAttachTypeToString(attachType);
    }

    /**
     * @brief Determines whether another instance can be created.
     *
     * @return True if creation is allowed, false if constrained by attachment.
     */
    [[nodiscard]] bool canCreateAnother() const
    {
        return !doesAttach();
    }

    /**
     * Creates a copy of the relation, but with a new name
     * @param newName The new name
     * @return A new CardRelation
     */
    [[nodiscard]] CardRelation copyWithName(const QString &newName) const;
};

#endif // COCKATRICE_CARD_RELATION_H
