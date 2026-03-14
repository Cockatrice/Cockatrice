/**
 * @file z_value_layer_manager.h
 * @ingroup GameGraphics
 * @brief Semantic Z-value layer management for game scene rendering.
 *
 * This file provides a structured approach to Z-value allocation in the game scene.
 * Z-values in Qt determine stacking order - higher values render on top of lower values.
 *
 * ## Layer Architecture
 *
 * The game scene is organized into three conceptual layers:
 *
 * 1. **Zone Layer (0-999)**: Zone backgrounds, containers, and static elements
 *    - Zone backgrounds (0.5-1.0)
 *    - Cards within zones (1.0 base + index)
 *
 * 2. **Card Layer (1-40,000,000)**: Dynamic card rendering on the table zone
 *    - Cards use formula: (actualY + CardDimensions::HEIGHT) * 100000 + (actualX + 1) * 100
 *    - Maximum card Z-value: ~40,000,000 (with 3 rows, actualY <= ~289)
 *
 * 3. **Overlay Layer (2,000,000,000+)**: UI elements that must appear above all cards
 *    - Hovered cards (+1)
 *    - Arrows (+3)
 *    - Zone views (+4)
 *    - Drag items (+5, +6)
 *    - Top UI elements (+7)
 *
 * ## Design Rationale
 *
 * The large gap between card Z-values (max ~40M) and overlay base (2B) provides
 * safety margin for future table zone expansions while ensuring overlays always
 * render above cards regardless of table position.
 *
 * ## Usage
 *
 * Prefer using the semantic constants from ZValues namespace:
 * @code
 * card->setZValue(ZValues::HOVERED_CARD);
 * arrow->setZValue(ZValues::ARROWS);
 * @endcode
 *
 * Use validation functions to verify card Z-values during development:
 * @code
 * Q_ASSERT(ZValueLayerManager::isValidCardZValue(cardZ));
 * @endcode
 */

#ifndef Z_VALUE_LAYER_MANAGER_H
#define Z_VALUE_LAYER_MANAGER_H

#include <QtGlobal>

/**
 * @namespace ZValueLayerManager
 * @brief Utilities for Z-value validation and layer management.
 */
namespace ZValueLayerManager
{

/**
 * @enum Layer
 * @brief Semantic layer identifiers for Z-value allocation.
 *
 * These represent conceptual rendering layers, not actual Z-values.
 * Use the corresponding ZValues constants for actual rendering.
 */
enum class Layer
{
    /// Zone-level elements like backgrounds and containers
    Zone,
    /// Cards rendered in zones (uses sequential Z-values)
    Card,
    /// Temporary UI elements like hovered cards and drag items
    Overlay
};

/**
 * @brief Maximum Z-value a card can have on the table zone.
 *
 * Based on table zone formula: (actualY + CardDimensions::HEIGHT) * 100000 + (actualX + 1) * 100
 * With maximum 3 rows and CardDimensions::HEIGHT = 102, actualY <= ~289.
 * Maximum: (289 + 102) * 100000 + 100 * 100 = 39,110,000
 *
 * We use 40,000,000 as a safe upper bound with margin.
 */
constexpr qreal CARD_Z_VALUE_MAX = 40000000.0;

/**
 * @brief Base Z-value for overlay elements.
 *
 * Must exceed CARD_Z_VALUE_MAX to ensure overlays render above all cards.
 * The 50x margin (2B vs 40M) provides safety for future expansion.
 */
constexpr qreal OVERLAY_BASE = 2000000000.0;

/**
 * @brief Validates that a Z-value is within the valid card range.
 *
 * Cards should have Z-values between CARD_BASE (1.0) and CARD_Z_VALUE_MAX.
 * Values outside this range may interfere with overlay rendering.
 *
 * @param zValue The Z-value to validate
 * @return true if the Z-value is valid for a card
 */
[[nodiscard]] constexpr bool isValidCardZValue(qreal zValue)
{
    return zValue >= 1.0 && zValue <= CARD_Z_VALUE_MAX;
}

/**
 * @brief Validates that a Z-value is in the overlay layer.
 *
 * Overlay elements should have Z-values at or above OVERLAY_BASE.
 *
 * @param zValue The Z-value to validate
 * @return true if the Z-value is valid for an overlay element
 */
[[nodiscard]] constexpr bool isOverlayZValue(qreal zValue)
{
    return zValue >= OVERLAY_BASE;
}

/**
 * @brief Returns the Z-value for a specific overlay element.
 *
 * @param offset Offset from OVERLAY_BASE (0-7 for current elements)
 * @return The absolute Z-value for the overlay element
 */
[[nodiscard]] constexpr qreal overlayZValue(qreal offset)
{
    return OVERLAY_BASE + offset;
}

} // namespace ZValueLayerManager

#endif // Z_VALUE_LAYER_MANAGER_H
