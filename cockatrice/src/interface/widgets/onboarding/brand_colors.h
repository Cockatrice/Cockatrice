#ifndef BRAND_COLORS_H
#define BRAND_COLORS_H

#include <QColor>

/** @brief Cockatrice brand green.
 *
 * Single source of truth for the onboarding brand accent: it backs the
 * banner's shader-accent uniform as the curated fallback when the active
 * palette resolves no usable Highlight, and it preseads the wizard's
 * QuickSetupPanel so a freshly generated palette keeps the brand identity
 * until the user picks their own look. */
inline const QColor kCockatriceBrandGreen(0x8B, 0xDD, 0x6B);

#endif // BRAND_COLORS_H