#ifndef Z_VALUES_H
#define Z_VALUES_H

#include "z_value_layer_manager.h"

/**
 * @file z_values.h
 * @ingroup GameGraphics
 * @brief Centralized Z-value constants for rendering layer order.
 *
 * Z-values in Qt determine stacking order. Higher values render on top.
 * These constants define the visual layering hierarchy for the game scene.
 *
 * ## Layer Architecture
 *
 * See z_value_layer_manager.h for detailed documentation on the three-layer
 * architecture (Zone, Card, Overlay) and the rationale for Z-value choices.
 *
 * ## Quick Reference
 *
 * | Layer    | Z-Value Range    | Purpose                           |
 * |----------|------------------|-----------------------------------|
 * | Zone     | 0.5 - 1.0        | Zone backgrounds, containers      |
 * | Card     | 1.0 - 40,000,000 | Cards on table (position-based)   |
 * | Overlay  | 2,000,000,000+   | UI elements above all cards       |
 */

namespace ZValues
{

// Expose base for callers that need it
constexpr qreal OVERLAY_BASE = ZValueLayerManager::OVERLAY_BASE;

// Overlay layer Z-values for items that should appear above normal cards
constexpr qreal HOVERED_CARD = ZValueLayerManager::overlayZValue(1.0);
constexpr qreal ARROWS = ZValueLayerManager::overlayZValue(3.0);
constexpr qreal ZONE_VIEW_WIDGET = ZValueLayerManager::overlayZValue(4.0);
constexpr qreal DRAG_ITEM = ZValueLayerManager::overlayZValue(5.0);
constexpr qreal DRAG_ITEM_CHILD = ZValueLayerManager::overlayZValue(6.0);
constexpr qreal TOP_UI = ZValueLayerManager::overlayZValue(7.0);

/**
 * @brief Compute Z-value for child drag items based on hotspot position.
 *
 * When dragging multiple cards together, each child card needs a unique Z-value
 * to prevent Z-fighting (flickering/flashing). The Z-values are derived from
 * their position when grabbed to conserve original stacking. The formula encodes
 * 2D coordinates into a single value where X has higher weight, ensuring
 * deterministic visual stacking.
 *
 * @param hotSpotX The X coordinate of the grab position
 * @param hotSpotY The Y coordinate of the grab position
 * @return Unique Z-value for the child drag item
 */
[[nodiscard]] constexpr qreal childDragZValue(qreal hotSpotX, qreal hotSpotY)
{
    return DRAG_ITEM_CHILD + hotSpotX * 1000000 + hotSpotY * 1000 + 1000;
}

/**
 * @brief Compute Z-value for cards on the table zone based on position.
 *
 * Cards lower on the table (higher Y) render above cards higher up,
 * and cards to the right (higher X) render above cards to the left.
 * This creates natural visual stacking for overlapping cards.
 *
 * @param x The X coordinate of the card position
 * @param y The Y coordinate of the card position
 * @return Z-value for the card's table position
 */
[[nodiscard]] constexpr qreal tableCardZValue(qreal x, qreal y)
{
    constexpr qreal CARD_HEIGHT_FOR_Z = 102.0;
    return (y + CARD_HEIGHT_FOR_Z) * 100000.0 + (x + 1) * 100.0;
}

// Card layering (general architecture, not command-zone specific)
constexpr qreal CARD_BASE = 1.0;
constexpr qreal CARD_MAX = ZValueLayerManager::CARD_Z_VALUE_MAX;

} // namespace ZValues

#endif // Z_VALUES_H
