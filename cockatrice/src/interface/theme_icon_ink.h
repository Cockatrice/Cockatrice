/**
 * @file theme_icon_ink.h
 * @ingroup Interface
 */

#ifndef THEME_ICON_INK_H
#define THEME_ICON_INK_H

#include "theme_icon.h"

#include <QColor>

/**
 * @brief Turns a text color into the ink a glyph is painted with.
 *
 * A glyph is not a word. Body text is read in sentences, at a size where a 2px
 * stroke would be unusually heavy; a glyph is a 16px mark of the same stroke,
 * and antialiasing eats its edges, so the same color that reads as solid type
 * reads as a thin gray line beside it. Painting icons with `WindowText` verbatim
 * is what makes them recede.
 *
 * The rule here is deliberately small: give the glyph a little of the theme's
 * accent chroma, then hold it to a contrast floor against the surface it sits
 * on. The chroma is what makes it read as an icon rather than as leftover text;
 * the floor keeps a theme with a weak text color from producing a glyph that is
 * not legible. Neither step invents a color: both are derived from the palette
 * the theme already ships. An unauthored neutral family stops at the floor: the
 * live platform palette is the only source that knows the actual scheme, so a
 * chroma pinned in a palette file could belong to the other one.
 *
 * The rule only applies to the neutral label roles. A semantic tint
 * (`AppColor::Positive`, `QPalette::Highlight`, `QPalette::Link`) is a hue the
 * call site chose on purpose, so it is left alone.
 */
namespace ThemeIconInk
{
/**
 * @brief Whether @p hue belongs to the band the Positive or Negative family owns.
 *
 * A green accent is a Positive color and a red accent is a Negative one, whether
 * or not the theme means it that way. Neutral chrome must not borrow either hue,
 * or "settings" and "add" are painted the same color.
 */
bool isSemanticHue(int hue);

/** @brief The hue neutral chrome falls back to when the accent is semantic. */
int neutralChromeHue();

/**
 * @brief The harder of two surfaces to keep an ink legible on.
 *
 * Contrast is monotone in the luminance distance between an ink and its
 * surface, so the surface nearest @p ink is the one it reads worst on. Choosing
 * that one keeps a single authored ink above the floor on both.
 */
QColor harderSurface(const QColor &first, const QColor &second, const QColor &ink);

/**
 * @brief WCAG relative luminance of an opaque color.
 */
double relativeLuminance(const QColor &color);

/**
 * @brief WCAG contrast ratio between two opaque colors, from 1.0 to 21.0.
 */
double contrastRatio(const QColor &first, const QColor &second);

/**
 * @brief Mixes two colors, @p amount = 0 returning @p color and 1 returning @p target.
 *
 * RGB interpolation, with no gamma or hue-space correction: the inputs are
 * palette colors that already sit in sRGB, and the result has to be predictable
 * enough for a theme author to reason about it.
 */
QColor blend(const QColor &color, const QColor &target, double amount);

/**
 * @brief Pushes @p color away from @p background until it reaches @p minRatio.
 *
 * Moves @p color toward whichever pole, black or white, actually reaches
 * @p minRatio against @p background, so the hue survives the adjustment.
 * Returns @p color unchanged when it already meets the floor, which is the
 * common case for a shipped palette.
 *
 * @param color Color to adjust.
 * @param background Surface the color is drawn on.
 * @param minRatio Contrast floor, as a WCAG ratio.
 * @return The adjusted color; the nearer pole when even that cannot reach it.
 */
QColor reachContrast(const QColor &color, const QColor &background, double minRatio);

/**
 * @brief Accent share used by production callers of glyphInk().
 *
 * Exposed so the preview sheet and the tests name the value they pin instead of
 * repeating the literal, and so the default argument below has exactly one
 * source: the constant in theme_icon_ink.cpp.
 */
double defaultAccentMix();

/**
 * @brief Icon ink for a neutral glyph, given the palette it is drawn from.
 *
 * @param text Neutral text color, normally `QPalette::WindowText`.
 * @param surface Surface the glyph is drawn on, normally `QPalette::Window`.
 * @param accent The theme accent, normally `QPalette::Highlight`.
 * @param accentMix How much of the accent color to borrow. Defaults to
 *                  defaultAccentMix(); previews and tests sweep it to compare
 *                  strengths. Production passes 0 for an unauthored neutral
 *                  family, where the live platform palette is the only source
 *                  that knows the actual scheme.
 * @return The ink to resolve `currentColor` to, as an RGB color.
 */
QColor glyphInk(const QColor &text, const QColor &surface, const QColor &accent, double accentMix = defaultAccentMix());

/**
 * @brief Contrast floor a glyph that labels a control has to clear.
 *
 * 4.5:1, the Text-level WCAG ratio: an icon that is the only label for a button
 * is carrying text, and the icon guidelines in `.opencode/docs/ui-assets.md`
 * already ask for that when there is no visible label.
 */
double labelContrastFloor();

/**
 * @brief Applies an emphasis level to an already-hued glyph color.
 *
 * The three levels share one legibility floor and differ by chroma, because
 * contrast cannot order them: on a dark surface the highest-contrast tone is the
 * palest one, so a contrast target inverts the ladder. Primary is the most
 * chromatic member of its family, secondary keeps the color untouched (the
 * no-op the default needs), and quiet mutes the color to a fraction of its
 * saturation before solving it back to the same floor.
 *
 * @param color Hue-resolved glyph color.
 * @param surface Surface the glyph is drawn on.
 * @param emphasis How much attention the glyph claims.
 * @return The emphasized color, as an RGB color.
 */
QColor forEmphasis(const QColor &color, const QColor &surface, IconEmphasis emphasis);

/**
 * @brief The ink a disabled glyph is painted with, given the quiet level.
 *
 * A disabled glyph is the quiet ink receded further toward the surface and held
 * to a floor below the quiet one. The palette's own disabled text is a fixed
 * gray whose contrast varies with the surface, so on the darkest themes it
 * out-reads the quiet level and an enabled glyph looks more disabled than a
 * disabled one. Deriving the disabled ink from the quiet ink makes that order
 * structural instead of incidental.
 */
QColor disabledInk(const QColor &quiet, const QColor &surface);

/**
 * @brief Contrast floor the primary level reaches, so it stays legible.
 *
 * Primary is ordered by chroma, not contrast, so it shares the text floor
 * rather than clearing a higher one.
 */
double primaryContrastFloor();

/** @brief Contrast floor the quiet level reaches, shared with the other levels. */
double quietContrastFloor();
} // namespace ThemeIconInk

#endif // THEME_ICON_INK_H
