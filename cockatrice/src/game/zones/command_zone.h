/**
 * @file command_zone.h
 * @ingroup GameGraphicsZones
 * @brief Graphics layer for the command zone, used for Commander format.
 */

#ifndef COCKATRICE_COMMAND_ZONE_H
#define COCKATRICE_COMMAND_ZONE_H

#include "command_zone_state.h"
#include "command_zone_types.h"
#include "logic/command_zone_logic.h"
#include "select_zone.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(CommandZoneLog, "command_zone");

class ZoneToggleButton;

/**
 * @class CommandZone
 * @brief Graphics layer for command zones in Commander format games.
 *
 * Supports primary (always visible) and collapsible (partner, companion, background)
 * variants. Primary/Partner hold one card each; Companion/Background support multiple
 * cards for edge-case mechanics. Single cards display centered; multiple cards use
 * zigzag horizontal positioning with vertical stacking.
 *
 * @see CommandZoneState for visibility/height state machine
 * @see supportsMultipleCards() in command_zone_types.h
 */
class CommandZone : public SelectZone
{
    Q_OBJECT
private:
    CommandZoneState m_state;             ///< Pure state machine for visibility/height
    ZoneToggleButton *toggleButton;       ///< Only created for collapsible zones
    QGraphicsRectItem *cardClipContainer; ///< Clips cards to zone bounds when minimized

    /**
     * @brief Logical parent for Qt-sibling zones (Companion, Background).
     *
     * INVARIANT: A zone has EITHER a Qt parent CommandZone (Partner) OR this
     * logical parent, never both. tryMinimizeAbove() relies on this exclusivity.
     */
    CommandZone *logicalParentZone = nullptr;

    /**
     * @brief Logical child zones for sibling architecture.
     *
     * Tracks zones that are logically children but Qt siblings. Used by
     * shouldPreventCollapse() and tryAutoMinimize() to traverse the hierarchy.
     */
    QList<CommandZone *> logicalChildZones;

public:
    /**
     * @brief Constructs a CommandZone graphics item.
     * @param _logic Logic layer managing card data
     * @param _zoneHeight Zone height in pixels
     * @param _zoneType Type of command zone (Primary, Partner, Companion, Background)
     * @param parent Parent graphics item
     */
    CommandZone(CommandZoneLogic *_logic, int _zoneHeight, CommandZoneType _zoneType, QGraphicsItem *parent);
    ~CommandZone() override;

    /**
     * @brief Handles card drops, calculating insertion position from drop point.
     * @param dragItems Cards being dragged
     * @param startZone Source zone
     * @param dropPoint Drop position in local coordinates
     */
    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;

    /**
     * @brief Sets expanded/collapsed state of a collapsible zone.
     *
     * When collapsed, background and cards are hidden but toggle button remains.
     * When expanded, zone renders normally with centered cards.
     * No-op for Primary zone.
     *
     * @param expanded True to show zone content, false to collapse
     */
    void setExpanded(bool expanded);

    void toggleMinimized();
    [[nodiscard]] bool isMinimized() const;
    [[nodiscard]] bool isExpanded() const;
    [[nodiscard]] bool isCollapsed() const;
    [[nodiscard]] bool isPrimary() const;
    [[nodiscard]] CommandZoneType getZoneType() const
    {
        return m_state.getZoneType();
    }
    [[nodiscard]] qreal currentHeight() const;

    /**
     * @brief Sets the logical parent zone for sibling architecture.
     * @param parent The logical parent CommandZone, or nullptr for nested zones
     */
    void setLogicalParentZone(CommandZone *parent)
    {
        logicalParentZone = parent;
    }

    /**
     * @brief Gets the logical parent zone.
     * @return The logical parent, or nullptr if using Qt parent-child hierarchy
     */
    [[nodiscard]] CommandZone *getLogicalParentZone() const
    {
        return logicalParentZone;
    }

    /**
     * @brief Adds a logical child zone for sibling architecture.
     * @param child The zone that is logically a child but a Qt sibling
     */
    void addLogicalChildZone(CommandZone *child)
    {
        logicalChildZones.append(child);
    }

    /**
     * @brief Attempts to auto-minimize this zone or a child zone (depth-first).
     *
     * Tries children first (depth-first, bottom-up) to minimize the
     * deepest non-minimized zone. Used as a fallback safety-net in
     * rearrangeZones() for non-expansion triggers (e.g., window resize).
     *
     * For user-initiated zone expansion, tryMinimizeAbove() is preferred
     * as it minimizes the zone above instead of below.
     *
     * @return True if a zone was minimized, false if no minimization possible
     * @see tryMinimizeAbove()
     */
    bool tryAutoMinimize();

    /**
     * @brief Returns the nearest CommandZone ancestor in the graphics item hierarchy.
     * @return Pointer to parent CommandZone, or nullptr if none exists.
     */
    [[nodiscard]] CommandZone *getParentCommandZone() const;

    /**
     * @brief Attempts to minimize the zone directly above this one.
     *
     * Walks up the parent chain to find the first ancestor CommandZone that
     * can be minimized (is expanded and not already minimized). The Primary
     * zone can be minimized (reduced to 25% height) but cannot be collapsed.
     *
     * Recursion is bounded by zone hierarchy depth (max 4 zones).
     *
     * @return True if an ancestor zone was minimized, false if none could be.
     * @see tryAutoMinimize()
     */
    bool tryMinimizeAbove();

    /**
     * @brief Sets toggle button visibility.
     * @param visible Whether toggle should be visible
     */
    void setToggleButtonVisible(bool visible);

public slots:
    void toggleExpanded();

signals:
    void expandedChanged(bool isExpanded);
    void minimizedChanged(bool isMinimized);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void updateBg();

private:
    void repositionToggleButton();
    void applyStateChange(const StateChangeResult &result, bool updateTogglePosition = false);
    [[nodiscard]] bool shouldPreventCollapse() const;
};

#endif // COCKATRICE_COMMAND_ZONE_H
