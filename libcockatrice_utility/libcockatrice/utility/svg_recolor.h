/**
 * @file svg_recolor.h
 * @ingroup Core
 */

#ifndef SVG_RECOLOR_H
#define SVG_RECOLOR_H

#include <QByteArray>

/**
 * @brief Resolves the `currentColor` sentinel in authored SVG data.
 *
 * Qt's SVG renderer has no CSS cascade or ambient color context, so the
 * sentinel is not resolved by Qt. Callers pass the concrete color they want
 * (for example a QPalette role serialized to #RRGGBB) and receive SVG bytes
 * that are safe to hand to QSvgRenderer.
 *
 * Matching is a plain, case-insensitive substring search, not a CSS parse: an
 * identifier that merely starts with the sentinel (for example
 * `id="currentColorStop"`) is rewritten too. That is acceptable for the assets
 * this project authors, and keeps the resolver dependency-free.
 *
 * @param svg SVG source bytes, expected to be UTF-8. Invalid UTF-8 is preserved
 *            byte-for-byte, since the resolver never decodes the document.
 * @param color Concrete SVG color (for example "#ff00ff"), as ASCII bytes.
 * @return The recolored SVG source; the input is left untouched.
 */
QByteArray resolveSvgColorSentinel(const QByteArray &svg, const QByteArray &color);

/**
 * @brief Rewrites the value of every `stroke-width` attribute in authored SVG.
 *
 * A treatment can need a heavier or lighter stroke than the one authored for a
 * line icon at text size. Like the sentinel resolver, this rewriter works on the
 * bytes and understands only the XML attribute form, in either quote style
 * (`stroke-width="2"` or `stroke-width='2'`): a CSS declaration such as
 * `style="stroke-width:2"` is left untouched. Documents without the attribute
 * are returned unchanged.
 *
 * @param svg SVG source bytes, expected to be UTF-8. Invalid UTF-8 is preserved
 *            byte-for-byte.
 * @param width Replacement width in the document's user units.
 * @return The rewritten SVG source; the input is left untouched.
 */
QByteArray resolveSvgStrokeWidth(const QByteArray &svg, double width);

#endif // SVG_RECOLOR_H
