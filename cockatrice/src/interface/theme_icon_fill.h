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
 * semantic tones are tuned for text, not for filling a whole shape; the
 * dark-scheme tones in particular are desaturated and read washed out once a
 * disc or a glyph carries them. The badge and the sticker share one fill rule so
 * they sit next to each other with the same green or red.
 *
 * The rule needs no scheme switch: a tone whose value is already high is a
 * dark-scheme tone, so its gradient is pulled down. Filling a shape with a tone
 * that light is what reads pale.
 */
namespace ThemeIconFill
{
/**
 * @brief Returns the tone with a saturation floor, for a filled shape.
 *
 * The hue and value are left alone, so a light-scheme tone stays as dark as the
 * line glyph next to it, and an achromatic tone stays gray.
 *
 * @param tone Glyph color the treatment is rendered with.
 * @return The tone, saturated enough to carry a whole shape.
 */
QColor saturated(const QColor &tone);

/**
 * @brief True when @p tone is a light tone, as the dark scheme's palette is.
 *
 * The fill branches on this: a tone this light is darkened for a filled shape,
 * and the badge keeps its darkened ink on it instead of the light scheme's
 * white glyph.
 *
 * @param tone Glyph color the treatment is rendered with.
 */
bool isDarkSchemeTone(const QColor &tone);

/**
 * @brief Returns the top color of the fill gradient.
 * @param tone Glyph color the treatment is rendered with.
 * @return The lighter end of the gradient.
 */
QColor topColor(const QColor &tone);

/**
 * @brief Returns the bottom color of the fill gradient.
 * @param tone Glyph color the treatment is rendered with.
 * @return The darker end of the gradient.
 */
QColor bottomColor(const QColor &tone);
} // namespace ThemeIconFill

#endif // THEME_ICON_FILL_H
