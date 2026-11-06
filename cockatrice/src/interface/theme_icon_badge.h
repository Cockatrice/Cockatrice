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
 * A tint alone changes the color of a 2px line, but the UI drew ready and
 * not-ready states as a filled badge before the one-glyph cutover. The badge
 * treatment keeps that shape: a filled disc, a dark ring and a contrast glyph,
 * all derived from the tone the tint resolves to. The theme still owns the
 * color. Callers only choose between a line and a badge.
 *
 * A badge survives a busy toolbar and a tinted row better than a hairline, and
 * its glyph can be light or dark regardless of the surrounding text. The cost is
 * more attention per control, and the glyph color follows the scheme it is drawn
 * for instead of the window's text color.
 *
 * Everything here works in device pixels. The icon engine renders at the
 * device-pixel size it already computed and sets the pixmap's device pixel ratio
 * afterwards, so badge geometry must not assume logical pixels.
 */
namespace ThemeIconBadge
{
/**
 * @brief Glyph color for a badge disc.
 *
 * White on the light scheme, the saturated tone darkened to an outline-dark ink
 * on the dark scheme. The dark-scheme tones fill the disc light enough that
 * white would sit near 2.7:1 at its top, so that scheme keeps the darkened ink
 * the badge had before the filled treatments shared a fill. The dark ink is
 * derived from the tone, so the theme keeps control of both ends.
 */
QColor glyphColor(const QColor &tone);

/** @brief Size of the engraved glyph for a badge of @p pixelSize, in device pixels. */
QSize glyphSize(const QSize &pixelSize);

/**
 * @brief Renders a sentinel-carrying SVG as a badge.
 *
 * The disc fills the square canvas minus the ring with the same gradient the
 * sticker fill uses, the ring darkens the tone, and the sentinel is resolved to
 * the contrast glyph. The SVG is scaled to nearly the whole disc, because the
 * sentinel paths are padded inside their viewBox and a smaller box loses the
 * stroke at 16px.
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
