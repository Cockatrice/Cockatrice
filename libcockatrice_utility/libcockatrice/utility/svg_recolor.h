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

#endif // SVG_RECOLOR_H
