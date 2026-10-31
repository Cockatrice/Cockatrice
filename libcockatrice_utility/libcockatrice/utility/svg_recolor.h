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
 * @param svg UTF-8 SVG source. Every `currentColor` occurrence, in any case,
 *            is replaced.
 * @param color Concrete SVG color (for example "#ff00ff"), as ASCII bytes.
 * @return The recolored SVG source; the input is left untouched.
 */
QByteArray resolveSvgColorSentinel(const QByteArray &svg, const QByteArray &color);

#endif // SVG_RECOLOR_H
