/**
 * @file theme_icon_badge.h
 * @ingroup Interface
 */

#ifndef THEME_ICON_BADGE_H
#define THEME_ICON_BADGE_H

#include <QByteArray>
#include <QColor>
#include <QPixmap>
#include <QSize>

/**
 * @brief Paints the badge treatment of a `currentColor` glyph.
 *
 * A tint alone can say "positive" or "negative", but a 2px line at 16px reads as
 * a different thing from the filled badge art the UI used before the one-glyph
 * cutover. The badge treatment keeps that shape: a filled disc, a dark ring, and
 * a contrast glyph, all derived from the one tone the tint resolves to. So the
 * theme still owns the color, and callers only choose between a line and a
 * badge.
 *
 * The trade-off is fidelity against contrast. A badge is a small picture: it
 * survives a busy toolbar and a tinted row better than a hairline, and it can
 * carry a light or dark glyph independent of the surrounding text. In exchange
 * it costs more pixels of attention per control, and its glyph color is decided
 * from the disc it sits on rather than from the window, so it is deliberately
 * not the palette's text color.
 *
 * Everything here works in device pixels. The icon engine renders at the
 * device-pixel size it already computed and sets the pixmap's device pixel ratio
 * afterwards, so badge geometry must not assume logical pixels.
 */
namespace ThemeIconBadge
{
/**
 * @brief WCAG relative luminance, used to test a color against a background.
 *
 * Shared by the badge glyph choice and by IconTint::resolve()'s selected-row
 * contrast test, so both use the same definition of "legible".
 */
double relativeLuminance(const QColor &color);

/** @brief WCAG contrast ratio between two opaque colors, from 1.0 to 21.0. */
double contrastRatio(const QColor &first, const QColor &second);

/**
 * @brief Glyph color for a badge disc.
 *
 * The more legible of white and the tone darkened to an outline-dark ink. White
 * keeps the old light-scheme look; on a dark scheme the semantic tone is a light
 * green or red, where white sits near 1.7:1 and disappears, so the dark ink
 * wins instead. Both candidates derive from the theme, so no literal is needed.
 */
QColor glyphColor(const QColor &tone);

/** @brief Size of the engraved glyph for a badge of @p pixelSize, in device pixels. */
QSize glyphSize(const QSize &pixelSize);

/**
 * @brief Renders a sentinel-carrying SVG as a badge.
 *
 * The disc fills the square canvas minus the ring, the ring darkens the tone,
 * and the sentinel is resolved to the contrast glyph. The SVG is scaled to
 * nearly the whole disc, because the sentinel paths are padded inside their
 * viewBox and a smaller box loses the stroke at 16px.
 *
 * This treatment reproduces the pre-#7595 ready/not-ready art, which was a
 * filled disc. It is not a generic replacement for every semantic glyph: the
 * old add/remove icons were filled glyphs with an outline and no disc, so
 * badging them would invent a look the UI never had.
 *
 * @param svg Sentinel-carrying SVG source; `currentColor` is resolved internally.
 * @param tone Disc color, already resolved for the icon mode.
 * @param pixelSize Canvas size in device pixels; a non-square canvas centers
 *                  the badge and leaves the remaining area transparent.
 * @return The composited badge, or a null pixmap for empty or invalid input.
 */
QPixmap renderBadge(const QByteArray &svg, const QColor &tone, const QSize &pixelSize);
} // namespace ThemeIconBadge

#endif // THEME_ICON_BADGE_H
