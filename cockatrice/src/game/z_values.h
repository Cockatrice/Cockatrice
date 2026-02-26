#ifndef Z_VALUES_H
#define Z_VALUES_H

#include "board/abstract_card_item.h"
#include "z_value_layer_manager.h"

#include <QColor>

/**
 * @file z_values.h
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
 * | Overlay  | 200,000,000+     | UI elements above all cards       |
 */

namespace ZValues
{

// ============================================================================
// Zone-Level Layering (relative to parent containers)
// ============================================================================

/// Toggle button renders above zone background but below cards (child of partner zone)
constexpr qreal TOGGLE_BUTTON = 0.5;

/// Partner zone level within command zone
constexpr qreal PARTNER_ZONE = 1.0;

/// Command zone sits at standard zone level
constexpr qreal COMMAND_ZONE = 1.0;

// ============================================================================
// Card Layering (cards use sequential indices starting from this base)
// ============================================================================

/// Base Z-value for cards in zones (actual Z = CARD_BASE + index)
constexpr qreal CARD_BASE = 1.0;

/// Maximum Z-value for cards on the table zone (see ZValueLayerManager for formula)
constexpr qreal CARD_MAX = ZValueLayerManager::CARD_Z_VALUE_MAX;

// ============================================================================
// Overlay Elements (always on top of regular game elements)
// ============================================================================

/// Base z-value for overlay elements. Uses ZValueLayerManager::OVERLAY_BASE.
/// Must exceed CARD_MAX to ensure overlays render above all cards.
constexpr qreal OVERLAY_BASE = ZValueLayerManager::OVERLAY_BASE;

/// Hovered card temporarily promotes to this layer
constexpr qreal HOVERED_CARD = ZValueLayerManager::overlayZValue(1.0);

/// Commander and partner tax counters overlay
constexpr qreal TAX_COUNTERS = ZValueLayerManager::overlayZValue(2.0);

/// Targeting arrows between cards/players
constexpr qreal ARROWS = ZValueLayerManager::overlayZValue(3.0);

/// Zone view widget (deck/graveyard viewer)
constexpr qreal ZONE_VIEW_WIDGET = ZValueLayerManager::overlayZValue(4.0);

/// Drag items and search UI elements
constexpr qreal DRAG_ITEM = ZValueLayerManager::overlayZValue(5.0);

/// Child drag items (stacked above parent drag)
constexpr qreal DRAG_ITEM_CHILD = ZValueLayerManager::overlayZValue(6.0);

/// Highest layer - group selectors
constexpr qreal TOP_UI = ZValueLayerManager::overlayZValue(7.0);

} // namespace ZValues

// ============================================================================
// Zone Size Constants
// ============================================================================

/**
 * @namespace ZoneSizes
 * @brief Centralized size constants for game zone dimensions.
 *
 * These constants define the dimensions of various zones to ensure
 * consistent sizing across the codebase and prevent duplication.
 */
namespace ZoneSizes
{

/// Height of the command zone (accommodates a card plus padding)
constexpr int COMMAND_ZONE_HEIGHT = CARD_HEIGHT + 8;

/// Minimum height for the stack zone to ensure usability
constexpr int MINIMUM_STACK_HEIGHT = 100;

/// Size of commander tax counter icons
constexpr int TAX_COUNTER_SIZE = 24;

/// Width of the command zone
constexpr qreal COMMAND_ZONE_WIDTH = 100.0;

/// Height ratio when command zone is minimized (25%)
constexpr qreal MINIMIZED_HEIGHT_RATIO = 0.25;

} // namespace ZoneSizes

// ============================================================================
// Game Color Constants
// ============================================================================

/**
 * @namespace GameColors
 * @brief Centralized color constants for overlay UI elements.
 *
 * These constants ensure visual consistency between related UI elements
 * like the partner zone toggle button and commander tax counters.
 */
namespace GameColors
{

/// Alpha value for overlay elements (75% opaque)
constexpr int OVERLAY_ALPHA = 191;

/// Background colors for overlay elements
inline const QColor OVERLAY_BG_NORMAL{40, 40, 40, OVERLAY_ALPHA};
inline const QColor OVERLAY_BG_HOVERED{70, 70, 70, OVERLAY_ALPHA};

} // namespace GameColors

#endif // Z_VALUES_H
