/**
 * @file theme_icon_sticker.h
 * @ingroup Interface
 */

#ifndef THEME_ICON_STICKER_H
#define THEME_ICON_STICKER_H

#include <QByteArray>
#include <QColor>
#include <QPixmap>
#include <QSize>

/**
 * @brief Paints the sticker treatment of a `currentColor` glyph.
 *
 * The pre-#7595 add, remove and delete icons were filled shapes with a black
 * outline and no disc. A tint alone cannot reproduce that: it produces a
 * hairline, and a badge disc is a different icon language. The sticker
 * treatment rebuilds the look from the same sentinel art by drawing the glyph
 * twice, once with a dark outline and once with a vertical tone gradient on top.
 *
 * A sticker is louder than a line, which suits an add or a destructive action.
 * It needs a size where the tone band is at least two device pixels wide.
 * Callers opt in. Navigation and menus keep the line treatment.
 *
 * Everything here works in device pixels, like the badge treatment: the engine
 * renders at the size it already computed and attaches the device pixel ratio
 * afterwards.
 */
namespace ThemeIconSticker
{
/**
 * @brief Returns the dark rim color the sticker is painted with.
 *
 * The old art had a black outline. Deriving it from the tone reads as black on a
 * light scheme without a literal, and stays visible on a dark one, where a pure
 * black rim would disappear into the window.
 *
 * @param tone Glyph color the sticker is rendered with.
 * @return A much darker version of the tone.
 */
QColor outlineColor(const QColor &tone);

/**
 * @brief Renders a sentinel-carrying SVG as a filled glyph with a black outline.
 *
 * The glyph is drawn larger than the canvas so the padded sentinel paths fill
 * the icon the way the old art did, with a vertical tone gradient as the fill
 * and a near-black rim. Authored art is never drawn through this path.
 *
 * @param svg Sentinel-carrying SVG source; `currentColor` is resolved internally.
 * @param tone Glyph color, already resolved for the icon mode.
 * @param pixelSize Canvas size in device pixels; a non-square canvas centers
 *                  the glyph and leaves the remaining area transparent.
 * @return The composited sticker, or a null pixmap for empty or invalid input.
 */
QPixmap renderSticker(const QByteArray &svg, const QColor &tone, const QSize &pixelSize);
} // namespace ThemeIconSticker

#endif // THEME_ICON_STICKER_H
