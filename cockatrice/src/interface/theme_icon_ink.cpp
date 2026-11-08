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
// A primary glyph has to out-read the default one on both schemes. The accent is
// a mid-tone: on a light surface it is *lighter* than the neutral ink, and on a
// dark surface it is *darker*, so neither "the accent as-is" nor "a contrast
// floor shared with the default" keeps the primary above the secondary. A 7:1
// target does: it darkens the accent on light schemes and lightens it on dark
// ones, so the primary level always out-contrasts the default level.
constexpr double PRIMARY_CONTRAST = 7.0;
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
    // Move away from the background: toward whichever pole actually reaches
    // further. Black and white cross at a luminance of about 0.18, not 0.5, so
    // the midpoint test would pick white on a surface where black is far more
    // legible -- and could drag an ink that already cleared the floor to white.
    // The hue is kept by interpolating, not by replacing.
    const QColor black(Qt::black);
    const QColor white(Qt::white);
    const QColor pole = contrastRatio(black, background) >= contrastRatio(white, background) ? black : white;
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

QColor forEmphasis(const QColor &color, const QColor &surface, IconEmphasis emphasis)
{
    switch (emphasis) {
        case IconEmphasis::Primary:
            return reachContrast(color, surface, PRIMARY_CONTRAST).toRgb();
        case IconEmphasis::Quiet:
            return reachContrast(blend(color, surface, QUIET_BLEND), surface, QUIET_CONTRAST_FLOOR).toRgb();
        case IconEmphasis::Secondary:
            break;
    }
    return color;
}

double primaryContrastFloor()
{
    return PRIMARY_CONTRAST;
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
