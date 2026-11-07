#include "theme_icon_ink.h"

#include <QtGlobal>
#include <cmath>

namespace
{
// How much of the theme accent a neutral glyph borrows. A quarter still reads as
// near-black at 16-24px, which is what made the first cut look unchanged, so the
// glyph takes most of the accent's hue. It stops short of the accent itself: the
// glyph keeps its own contrast against the surface instead of becoming a second
// accent fill. Every shipped theme accents in blue or violet, so the borrowed hue
// stays neutral chrome instead of implying a state.
constexpr double ACCENT_MIX = 0.70;
// An icon that labels a control is carrying text; see ui-assets.md.
constexpr double LABEL_CONTRAST_FLOOR = 4.5;
// A primary glyph is the most chromatic member of its family. Contrast cannot
// order the ladder: on a dark surface the highest-contrast tone is the palest
// one, so a contrast target makes the primary look like a washed-out secondary
// and the ladder reads inverted. Primary instead raises the saturation by this
// many HSV points over the secondary and then solves the value back to the text
// floor, so it is louder in color at the same legibility.
constexpr int PRIMARY_SATURATION_STEP = 64;
// A quiet glyph is blended this far into its surface. It has to stay findable
// without competing with the actions beside it.
constexpr double QUIET_BLEND = 0.55;
// WCAG 2.2 SC 1.4.11, the floor for a non-text element that carries meaning.
constexpr double QUIET_CONTRAST_FLOOR = 3.0;
// Binary-search depth for reachContrast(). 24 halvings resolve to well under
// one 8-bit channel step, so the result is stable across platforms.
constexpr int CONTRAST_SEARCH_STEPS = 24;
} // namespace

namespace ThemeIconInk
{
double relativeLuminance(const QColor &color)
{
    const auto linear = [](double channel) {
        channel /= 255.0;
        return channel <= 0.03928 ? channel / 12.92 : std::pow((channel + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear(color.red()) + 0.7152 * linear(color.green()) + 0.0722 * linear(color.blue());
}

double contrastRatio(const QColor &first, const QColor &second)
{
    const double a = relativeLuminance(first);
    const double b = relativeLuminance(second);
    return (qMax(a, b) + 0.05) / (qMin(a, b) + 0.05);
}

QColor blend(const QColor &color, const QColor &target, double amount)
{
    const double clamped = qBound(0.0, amount, 1.0);
    const auto channel = [clamped](int from, int to) { return qRound(from + (to - from) * clamped); };
    return QColor(channel(color.red(), target.red()), channel(color.green(), target.green()),
                  channel(color.blue(), target.blue()));
}

QColor reachContrast(const QColor &color, const QColor &background, double minRatio)
{
    if (contrastRatio(color, background) >= minRatio) {
        return color;
    }
    // Move away from the background: toward black on a light surface, toward
    // white on a dark one. The hue is kept by interpolating, not by replacing.
    const QColor pole = relativeLuminance(background) > 0.5 ? QColor(Qt::black) : QColor(Qt::white);
    if (contrastRatio(pole, background) < minRatio) {
        return pole;
    }

    // The ratio is monotone along the path from color to pole, so a bisection
    // finds the first point that clears the floor. Stepping the opposite way
    // from the pole keeps the glyph as close to its palette color as possible.
    double low = 0.0;
    double high = 1.0;
    for (int step = 0; step < CONTRAST_SEARCH_STEPS; ++step) {
        const double middle = (low + high) / 2.0;
        if (contrastRatio(blend(color, pole, middle), background) >= minRatio) {
            high = middle;
        } else {
            low = middle;
        }
    }
    return blend(color, pole, high);
}

QColor glyphInk(const QColor &text, const QColor &surface, const QColor &accent, double accentMix)
{
    return reachContrast(blend(text, accent, accentMix), surface, LABEL_CONTRAST_FLOOR).toRgb();
}

double defaultAccentMix()
{
    return ACCENT_MIX;
}

double labelContrastFloor()
{
    return LABEL_CONTRAST_FLOOR;
}

namespace
{
/** @brief The luminance a color needs to reach @p ratio against @p surface. */
double targetLuminance(const QColor &surface, double ratio)
{
    const double surfaceLuminance = ThemeIconInk::relativeLuminance(surface);
    return surfaceLuminance > 0.5 ? (surfaceLuminance + 0.05) / ratio - 0.05 : (surfaceLuminance + 0.05) * ratio - 0.05;
}

/**
 * @brief Raises a color's saturation, then solves its value for the floor.
 *
 * Saturation is raised before the value is solved, because reaching a contrast
 * floor by adding white is what desaturates a tone in the first place. If the
 * hue cannot reach the floor at the raised saturation inside the gamut, the step
 * shrinks until it can, so an already-vivid tone keeps what it has. A gray has
 * no saturation to raise and falls back to the contrast search.
 */
QColor chromaticStep(const QColor &color, const QColor &surface, double floor, int stepPoints)
{
    if (color.hue() < 0) {
        return ThemeIconInk::reachContrast(color, surface, floor);
    }

    const double target = targetLuminance(surface, floor);
    const int hue = color.hue();
    const int secondary = color.hsvSaturation();
    const int primary = qMin(255, secondary + stepPoints);
    for (int saturation = primary; saturation > secondary; --saturation) {
        if (ThemeIconInk::relativeLuminance(QColor::fromHsv(hue, saturation, 255)) < target) {
            continue;
        }
        int low = 0;
        int high = 255;
        for (int search = 0; search < CONTRAST_SEARCH_STEPS; ++search) {
            const int middle = (low + high) / 2;
            if (ThemeIconInk::relativeLuminance(QColor::fromHsv(hue, saturation, middle)) >= target) {
                high = middle;
            } else {
                low = middle;
            }
        }
        return QColor::fromHsv(hue, saturation, high).toRgb();
    }
    return ThemeIconInk::reachContrast(color, surface, floor);
}
} // namespace

QColor forEmphasis(const QColor &color, const QColor &surface, IconEmphasis emphasis)
{
    switch (emphasis) {
        case IconEmphasis::Primary:
            return chromaticStep(color, surface, LABEL_CONTRAST_FLOOR, PRIMARY_SATURATION_STEP).toRgb();
        case IconEmphasis::Quiet:
            return reachContrast(blend(color, surface, QUIET_BLEND), surface, QUIET_CONTRAST_FLOOR).toRgb();
        case IconEmphasis::Secondary:
            break;
    }
    return color;
}

double primaryContrastFloor()
{
    return LABEL_CONTRAST_FLOOR;
}

double quietBlend()
{
    return QUIET_BLEND;
}

double quietContrastFloor()
{
    return QUIET_CONTRAST_FLOOR;
}
} // namespace ThemeIconInk
