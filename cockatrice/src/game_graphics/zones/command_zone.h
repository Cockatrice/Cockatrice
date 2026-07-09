/**
 * @file command_zone.h
 * @ingroup GameGraphicsZones
 * @brief Graphics layer for the command zone, used for Commander format.
 */

#ifndef COCKATRICE_COMMAND_ZONE_H
#define COCKATRICE_COMMAND_ZONE_H

#include "../../game/zones/command_zone_logic.h"
#include "../card_dimensions.h"
#include "select_zone.h"

#include <QLoggingCategory>

class AbstractCounter;

inline Q_LOGGING_CATEGORY(CommandZoneLog, "command_zone");

/**
 * @namespace ZoneSizes
 * @brief Size constants for the command zone and its sub-elements.
 */
namespace ZoneSizes
{

/** @brief Height of the command zone (accommodates a card plus padding) */
constexpr qreal COMMAND_ZONE_HEIGHT = CardDimensions::HEIGHT + 8;

/** @brief Width of the command zone (matches stack zone) */
constexpr qreal COMMAND_ZONE_WIDTH = CardDimensions::WIDTH_F * 1.5;

} // namespace ZoneSizes

/**
 * @class CommandZone
 * @brief Graphics layer for the command zone in Commander format games.
 *
 * Always visible when enabled. Supports multiple cards using a zigzag
 * horizontal stacking pattern: single cards display centered, multiple
 * cards alternate left-right with vertical overlap compression.
 * Can be minimized to 25% height via double-click.
 *
 * @see CommandZoneLogic for card data management
 * @see CommanderTaxCounter for the tax counter overlay
 */
class CommandZone : public SelectZone
{
    Q_OBJECT
private:
    static constexpr double MINIMIZED_HEIGHT_RATIO = 0.25;
    int zoneHeight;                       ///< Full height in pixels when expanded
    bool minimized = false;               ///< Whether zone is at 25% height
    int minimumHeight = 0;                ///< Floor for minimized height (e.g. to fit tax counters)
    QList<AbstractCounter *> taxCounters; ///< Registered tax counter widgets

public:
    /**
     * @brief Constructs a CommandZone graphics item.
     * @param _logic Logic layer managing card data
     * @param _zoneHeight Zone height in pixels
     * @param parent Parent graphics item
     */
    CommandZone(CommandZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent);
    ~CommandZone() override;

    /**
     * @brief Handles card drops, calculating insertion position from drop point.
     * @param dragItems Cards being dragged
     * @param startZone Source zone
     * @param dropPoint Drop position in local coordinates
     */
    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;

    /** @brief Returns the bounding rectangle, accounting for minimized state. */
    [[nodiscard]] QRectF boundingRect() const override;
    /** @brief Paints the zone background using the Commander theme brush. */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /** @brief Repositions cards using zigzag horizontal stacking with overlap compression. */
    void reorganizeCards() override;

    /** @brief Toggles between full and 25% minimized height. */
    void toggleMinimized();
    [[nodiscard]] bool isMinimized() const;
    /** @brief Returns the current display height (full or minimized). */
    [[nodiscard]] qreal currentHeight() const;
    /** @brief Sets the minimum height floor, e.g. to ensure tax counters remain visible. */
    void setMinimumHeight(int height);
    /** @brief Registers a tax counter widget for layout management. */
    void registerTaxCounter(AbstractCounter *counter);
    /**
     * @brief Unregisters a tax counter widget from layout management.
     *
     * Call on removal: the widget outlives it via deleteLater(), so unregister synchronously
     * to keep it out of the next layout pass.
     */
    void unregisterTaxCounter(AbstractCounter *counter);
    /** @brief Lays out visible tax counters vertically in the top-left corner of the command zone. */
    void rearrangeTaxCounters();

signals:
    void minimizedChanged(bool isMinimized);
    // Displayed height changed without a minimized-state change (e.g. tax counter toggled
    // while minimized); lets neighbouring zones reposition.
    void effectiveHeightChanged();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private slots:
    void updateBg();
};

#endif // COCKATRICE_COMMAND_ZONE_H
