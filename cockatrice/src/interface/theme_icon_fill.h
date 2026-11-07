/**
 * @file theme_icon_fill.h
 * @ingroup Interface
 */

#ifndef THEME_ICON_FILL_H
#define THEME_ICON_FILL_H

#include <QColor>

/**
 * @brief Picks the two colors of a filled treatment's vertical gradient.
 *
 * The pre-#7595 icons were filled with a lighter top over a darker bottom. The
 * badge and the sticker share one fill rule so they sit next to each other with
 * the same green or red.
 *
 * The authored tone is the top of that gradient: the palette author stores one
 * color per emphasis level, so the treatment only adds depth by darkening the
 * bottom. Hue, saturation and value all stay the author's, so the rendered
 * ladder is the authored ladder on every scheme.
 */
namespace ThemeIconFill
{
/**
 * @brief Which scheme a treatment is painted for.
 *
 * The scheme is a property of the surface, not of the tone: an authored quiet
 * level is deliberately darker than its secondary level on the dark scheme too,
 * so it cannot be inferred from the tone's value. Callers know the surface
 * palette and pass it explicitly.
 */
enum class Scheme
{
    Light,
    Dark,
};

/**
 * @brief Returns the top color of the fill gradient.
 * @param tone Glyph color the treatment is rendered with.
 * @return The authored tone, unchanged.
 */
QColor topColor(const QColor &tone);

/**
 * @brief Returns the bottom color of the fill gradient.
 * @param tone Glyph color the treatment is rendered with.
 * @return The authored tone, darkened for depth.
 */
QColor bottomColor(const QColor &tone);
} // namespace ThemeIconFill

#endif // THEME_ICON_FILL_H
