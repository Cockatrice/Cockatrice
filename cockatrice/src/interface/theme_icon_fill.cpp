#include "theme_icon_fill.h"

#include <QtGlobal>

namespace
{
// Semantic tones are tuned for text. The dark-scheme ones are deliberately
// desaturated, which reads washed out when the same color fills a whole shape,
// so the fill gives its tone a saturation floor.
constexpr int MIN_SATURATION = 170;
// A tone this light belongs to the dark scheme; filling a glyph with it at full
// value is what looks pale, so its gradient is pulled down instead of up.
constexpr int LIGHT_TONE_VALUE = 200;
constexpr int MIN_VALUE = 200;
constexpr int GRADIENT_BOTTOM_DARKER = 140;
constexpr int DARK_SCHEME_TOP_DARKER = 120;
constexpr int DARK_SCHEME_BOTTOM_DARKER = 170;
} // namespace

namespace
{
/** @brief Raises a tone's saturation, and optionally its value, for a filled shape. */
QColor boost(const QColor &color, int minValue)
{
    QColor result = color;
    if (color.hue() >= 0) {
        result.setHsv(color.hue(), qMax(color.hsvSaturation(), MIN_SATURATION), qMax(color.value(), minValue));
    }
    return result;
}
} // namespace

namespace ThemeIconFill
{
QColor saturated(const QColor &tone)
{
    return boost(tone, 0);
}

QColor topColor(const QColor &tone)
{
    if (tone.value() >= LIGHT_TONE_VALUE) {
        return boost(tone, 0).darker(DARK_SCHEME_TOP_DARKER);
    }
    return boost(tone, MIN_VALUE);
}

QColor bottomColor(const QColor &tone)
{
    const int darker = tone.value() >= LIGHT_TONE_VALUE ? DARK_SCHEME_BOTTOM_DARKER : GRADIENT_BOTTOM_DARKER;
    return boost(tone, 0).darker(darker);
}
} // namespace ThemeIconFill
