#ifndef COCKATRICE_CARD_RELATION_H
#define COCKATRICE_CARD_RELATION_H

#include "card_relation_type.h"

#include <QObject>
#include <QString>

/**
 * @class CardRelation
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
class CardRelation : public QObject
{
    Q_OBJECT

private:
    QString name;                ///< Name of the related card.
    CardRelationType attachType; ///< Type of attachment.
    bool isCreateAllExclusion; ///< True if this relation should exclude multiple creations in "create all" operations.
    bool isVariableCount;      ///< True if the number of creations is variable.
    int defaultCount;          ///< Default number of cards created or involved.
    bool isPersistent;         ///< True if this relation persists (i.e. is not destroyed) on zone change.

public:
    /**
     * @brief Constructs a CardRelation with optional parameters.
     *
     * @param _name Name of the related card.
     * @param _attachType Type of attachment.
     * @param _isCreateAllExclusion Whether this relation excludes mass creation.
     * @param _isVariableCount Whether the count is variable.
     * @param _defaultCount Default number for creations or transformations.
     * @param _isPersistent Whether the relation persists across zone changes.
     */
    explicit CardRelation(const QString &_name = QString(),
                          CardRelationType _attachType = CardRelationType::DoesNotAttach,
                          bool _isCreateAllExclusion = false,
                          bool _isVariableCount = false,
                          int _defaultCount = 1,
                          bool _isPersistent = false);

    /**
     * @brief Returns the name of the related card.
     *
     * @return Name as QString reference.
     */
    [[nodiscard]] inline const QString &getName() const
    {
        return name;
    }

    /**
     * @brief Returns the type of attachment.
     *
     * @return Enum value representing the attachment type.
     */
    [[nodiscard]] CardRelationType getAttachType() const
    {
        return attachType;
    }

    /**
     * @brief Returns true if the card is attached to another.
     *
     * @return True if attached, false otherwise.
     */
    [[nodiscard]] bool getDoesAttach() const
    {
        return attachType != CardRelationType::DoesNotAttach;
    }

    /**
     * @brief Returns true if this card transforms into another card.
     *
     * @return True if it transforms, false otherwise.
     */
    [[nodiscard]] bool getDoesTransform() const
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
    [[nodiscard]] bool getCanCreateAnother() const
    {
        return !getDoesAttach();
    }

    /**
     * @brief Returns whether this relation is excluded from "create all" operations.
     *
     * @return True if excluded, false otherwise.
     */
    [[nodiscard]] bool getIsCreateAllExclusion() const
    {
        return isCreateAllExclusion;
    }

    /**
     * @brief Returns whether the relation count is variable.
     *
     * @return True if variable, false otherwise.
     */
    [[nodiscard]] bool getIsVariable() const
    {
        return isVariableCount;
    }

    /**
     * @brief Returns the default count of related cards.
     *
     * @return Integer representing default number.
     */
    [[nodiscard]] int getDefaultCount() const
    {
        return defaultCount;
    }

    /**
     * @brief Returns whether the relation is persistent.
     *
     * Persistent relations are not destroyed on zone changes.
     *
     * @return True if persistent, false otherwise.
     */
    [[nodiscard]] bool getIsPersistent() const
    {
        return isPersistent;
    }
};

#endif // COCKATRICE_CARD_RELATION_H
