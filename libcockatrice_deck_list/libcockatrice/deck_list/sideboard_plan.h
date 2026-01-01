#ifndef COCKATRICE_SIDEBOARD_PLAN_H
#define COCKATRICE_SIDEBOARD_PLAN_H

#include <QList>
#include <libcockatrice/protocol/pb/move_card_to_zone.pb.h>

class QXmlStreamWriter;
class QXmlStreamReader;

/**
 * @class SideboardPlan
 * @ingroup Decks
 * @brief Represents a predefined sideboarding strategy for a deck.
 *
 * Sideboard plans store a named list of card movements that should be applied
 * between the mainboard and sideboard for a specific matchup. Each movement
 * is expressed using a `MoveCard_ToZone` protobuf message.
 *
 * ### Responsibilities:
 * - Store the plan name and list of moves.
 * - Support XML serialization/deserialization.
 *
 * ### Typical usage:
 * A deck can contain multiple sideboard plans (e.g., "vs Aggro", "vs Control"),
 * each describing how to transform the main deck into its intended configuration.
 */
class SideboardPlan
{
private:
    QString name;                    ///< Human-readable name of this plan.
    QList<MoveCard_ToZone> moveList; ///< List of move instructions for this plan.

public:
    /**
     * @brief Construct a new SideboardPlan.
     * @param _name The plan name.
     * @param _moveList Initial list of card move instructions.
     */
    explicit SideboardPlan(const QString &_name = "", const QList<MoveCard_ToZone> &_moveList = {});

    /**
     * @brief Read a SideboardPlan from an XML stream.
     * @param xml XML reader positioned at the plan element.
     * @return true if parsing succeeded.
     */
    bool readElement(QXmlStreamReader *xml);

    /**
     * @brief Write this SideboardPlan to XML.
     * @param xml Stream to append the serialized element to.
     */
    void write(QXmlStreamWriter *xml) const;

    /// @return The plan name.
    [[nodiscard]] QString getName() const
    {
        return name;
    }

    /// @return Const reference to the move list.
    [[nodiscard]] const QList<MoveCard_ToZone> &getMoveList() const
    {
        return moveList;
    }

    /// @brief Replace the move list with a new one.
    void setMoveList(const QList<MoveCard_ToZone> &_moveList);
};

#endif // COCKATRICE_SIDEBOARD_PLAN_H
