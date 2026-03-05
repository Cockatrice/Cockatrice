#ifndef CARD_DIMENSIONS_H
#define CARD_DIMENSIONS_H

#include <QtGlobal>

/**
 * @file card_dimensions.h
 * @brief Canonical card dimension constants for layout and Z-value calculations.
 *
 * These values represent the logical pixel dimensions of a standard card graphic.
 * They are used throughout the game scene for layout, rendering, and Z-value computation.
 */
namespace CardDimensions
{
/// Card width in pixels
constexpr int WIDTH = 72;
/// Card height in pixels
constexpr int HEIGHT = 102;

/// Pre-converted for floating-point contexts (Z-value calculations)
constexpr qreal WIDTH_F = static_cast<qreal>(WIDTH);
constexpr qreal HEIGHT_F = static_cast<qreal>(HEIGHT);
} // namespace CardDimensions

#endif // CARD_DIMENSIONS_H
