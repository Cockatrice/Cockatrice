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
// The Positive family owns green and the Negative family owns red. An accent in
// either band is a semantic color the theme did not mean as chrome, so the
// neutral glyph keeps the accent's intensity but not its hue.
constexpr int SEMANTIC_GREEN_START = 80;
constexpr int SEMANTIC_GREEN_END = 165;
constexpr int SEMANTIC_RED_START = 340;
constexpr int SEMANTIC_RED_END = 25;
// Slate blue: the chrome hue every shipped theme already leans toward, and the
// least likely to be mistaken for a state.
constexpr int NEUTRAL_CHROME_HUE = 210;
// An icon that labels a control is carrying text; see ui-assets.md.
constexpr double LABEL_CONTRAST_FLOOR = 4.5;
// A primary glyph is the most chromatic member of its family. Contrast cannot
// order the ladder: on a dark surface the highest-contrast tone is the palest
// one, so a contrast target makes the primary look like a washed-out secondary
// and the ladder reads inverted. Primary instead raises the saturation by this
// many HSV points over the secondary and then solves the value back to the text
// floor, so it is louder in color at the same legibility.
constexpr int PRIMARY_SATURATION_STEP = 64;
// Quiet recedes by carrying less of the family's chroma, not by dropping closer
// to the surface: the disabled level owns the low-contrast end. It keeps this
// share of the secondary level's saturation.
constexpr int QUIET_SATURATION_PERCENT = 45;
// The quiet level shares the text floor with the other two levels.
constexpr double QUIET_CONTRAST_FLOOR = LABEL_CONTRAST_FLOOR;
// A disabled glyph recedes past the quiet level, so it is blended this much
// further into the surface...
constexpr double DISABLED_BLEND = 0.55;
// ...and then held to a floor below the quiet one so it stays findable.
constexpr double DISABLED_CONTRAST_FLOOR = 2.5;
// 8-bit quantization can drop a solved color just under the ratio it was solved
// for, so every search aims this much above the floor. Without it a palette
// value generated at exactly 4.5:1 measures 4.45:1 after the round trip.
constexpr double CONTRAST_ROUNDING_MARGIN = 0.10;
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
    const double target = minRatio + CONTRAST_ROUNDING_MARGIN;
    if (contrastRatio(color, background) >= target) {
        return color;
    }
    // Move toward whichever pole the background leaves room for. A medium-light
    // surface is lighter than mid-gray but still too light to reach a text floor
    // with white, so "light background means move toward black" is the wrong
    // test: compare what each pole can actually reach.
    const double blackRatio = contrastRatio(QColor(Qt::black), background);
    const double whiteRatio = contrastRatio(QColor(Qt::white), background);
    const QColor pole = blackRatio >= whiteRatio ? QColor(Qt::black) : QColor(Qt::white);
    if (qMax(blackRatio, whiteRatio) < target) {
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

bool isSemanticHue(int hue)
{
    if (hue < 0) {
        return false;
    }
    return (hue >= SEMANTIC_GREEN_START && hue <= SEMANTIC_GREEN_END) || hue >= SEMANTIC_RED_START ||
           hue <= SEMANTIC_RED_END;
}

int neutralChromeHue()
{
    return NEUTRAL_CHROME_HUE;
}

QColor harderSurface(const QColor &first, const QColor &second, const QColor &ink)
{
    const double inkLuminance = relativeLuminance(ink);
    const auto distance = [inkLuminance](const QColor &surface) {
        return std::abs(relativeLuminance(surface) - inkLuminance);
    };
    return distance(first) <= distance(second) ? first : second;
}

QColor glyphInk(const QColor &text, const QColor &surface, const QColor &accent, double accentMix)
{
    // A green or red accent is a Positive or Negative color, so borrowing its
    // hue would paint neutral chrome the same color as a semantic state. Keep
    // the accent's intensity, replace its hue with the chrome hue.
    QColor source = accent;
    if (isSemanticHue(accent.hue())) {
        source = QColor::fromHsv(NEUTRAL_CHROME_HUE, accent.hsvSaturation(), qMax(accent.value(), 128));
    }
    return reachContrast(blend(text, source, accentMix), surface, LABEL_CONTRAST_FLOOR).toRgb();
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
/** @brief Whether black reaches a higher contrast against @p surface than white. */
bool darkerPoleWins(const QColor &surface)
{
    return ThemeIconInk::contrastRatio(QColor(Qt::black), surface) >=
           ThemeIconInk::contrastRatio(QColor(Qt::white), surface);
}

/** @brief The luminance a color needs to reach @p ratio against @p surface. */
double targetLuminance(const QColor &surface, double ratio)
{
    const double surfaceLuminance = ThemeIconInk::relativeLuminance(surface);
    return darkerPoleWins(surface) ? (surfaceLuminance + 0.05) / ratio - 0.05
                                   : (surfaceLuminance + 0.05) * ratio - 0.05;
}

/**
 * @brief The value that puts @p hue at @p saturation on @p surface's floor.
 *
 * Luminance is monotone in HSV value at a fixed hue and saturation, so a
 * bisection finds the boundary: the closest tone to the surface that still
 * clears the floor, on either scheme. Returns an invalid color when even full
 * value cannot reach the floor at that saturation.
 */
QColor solveValue(int hue, int saturation, const QColor &surface, double floor)
{
    const double target = targetLuminance(surface, floor + CONTRAST_ROUNDING_MARGIN);
    if (ThemeIconInk::relativeLuminance(QColor::fromHsv(hue, saturation, 255)) < target) {
        return {};
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

    const int hue = color.hue();
    const int secondary = color.hsvSaturation();
    for (int saturation = qMin(255, secondary + stepPoints); saturation > secondary; --saturation) {
        const QColor solved = solveValue(hue, saturation, surface, floor);
        if (solved.isValid()) {
            return solved;
        }
    }
    return ThemeIconInk::reachContrast(color, surface, floor);
}

/**
 * @brief Mutes a color's saturation, then solves its value for the floor.
 *
 * The mirror of chromaticStep: quiet stays on the same floor as the other
 * levels and recedes by carrying less of the family's chroma, so it cannot drop
 * below the disabled level's contrast by construction. A gray has no saturation
 * to mute and falls back to the contrast search.
 */
QColor mutedStep(const QColor &color, const QColor &surface, double floor, int percent)
{
    if (color.hue() < 0) {
        return ThemeIconInk::reachContrast(color, surface, floor);
    }

    const int saturation = qBound(0, qRound(color.hsvSaturation() * percent / 100.0), 255);
    const QColor solved = solveValue(color.hue(), saturation, surface, floor);
    return solved.isValid() ? solved : ThemeIconInk::reachContrast(color, surface, floor);
}
} // namespace

QColor forEmphasis(const QColor &color, const QColor &surface, IconEmphasis emphasis)
{
    switch (emphasis) {
        case IconEmphasis::Primary:
            return chromaticStep(color, surface, LABEL_CONTRAST_FLOOR, PRIMARY_SATURATION_STEP).toRgb();
        case IconEmphasis::Quiet:
            return mutedStep(color, surface, QUIET_CONTRAST_FLOOR, QUIET_SATURATION_PERCENT).toRgb();
        case IconEmphasis::Secondary:
            break;
    }
    return color;
}

QColor disabledInk(const QColor &quiet, const QColor &surface)
{
    return reachContrast(blend(quiet, surface, DISABLED_BLEND), surface, DISABLED_CONTRAST_FLOOR).toRgb();
}

double primaryContrastFloor()
{
    return LABEL_CONTRAST_FLOOR;
}

double quietContrastFloor()
{
    return QUIET_CONTRAST_FLOOR;
}
} // namespace ThemeIconInk
