#include "palette_generator.h"

#include <QColor>

// ════════════════════════════════════════════════════════════════════════════
//  PaletteGenerator::fromAccent
//
//  Three intensity bands:
//    0–30   Subtle   — Highlight / links / BrightText take on the hue;
//                      backgrounds stay neutral grey.
//    30–70  Accented — Button, ToolTipBase, AlternateBase, shading tint in;
//                      backgrounds get a faint hue wash.
//    70–100 Chromatic— Window and Base pick up real saturation; the whole
//                      application reads as that colour, text stays readable.
//
//  Key principles:
//  • Quadratic saturation curve: low end feels truly subtle, top is vivid.
//  • Each role has its own saturation ceiling; buttons always pop above window.
//  • Base stays near-white / near-black regardless of intensity for legibility.
//  • Button text contrast is computed from the real button color, not assumed.
//  • Tooltip blends from classic yellow to hue-tinted above intensity ~25.
//  • 3D shading ladder (Light→Shadow) carries the same hue for coherence.
//  • Disabled: text → mid-gray, backgrounds → match Window.
//  • Inactive Highlight fades to a near-bg tone so it doesn't compete.
// ════════════════════════════════════════════════════════════════════════════

namespace PaletteGenerator
{

PaletteConfig fromAccent(const QColor &accent, int intensity, const QString &scheme)
{
    PaletteConfig cfg;
    const bool dark = scheme.compare("Dark", Qt::CaseInsensitive) == 0;
    const double t = intensity / 100.0; // 0.0 – 1.0

    int h = accent.hslHue();
    const bool achromatic = (h < 0);
    if (achromatic) {
        h = 0;
    }

    // Saturation budgets
    // Quadratic ease-in means the subtle end is genuinely subtle and the
    // full end is bold without being garish.
    auto sat = [&](double maxSat) -> int {
        if (achromatic) {
            return 0;
        }
        return qRound(maxSat * t * t);
    };

    const int satWindow = sat(dark ? 80.0 : 90.0);
    const int satBase = sat(dark ? 25.0 : 20.0); // text areas stay near-white/black
    const int satAlt = sat(dark ? 90.0 : 100.0);
    const int satButton = sat(dark ? 120.0 : 130.0); // buttons pop above the bg
    const int satTooltip = sat(dark ? 90.0 : 80.0);
    const int satHighlight = achromatic ? 0 : qRound(accent.hslSaturation() * (0.45 + 0.55 * t));
    const int satShadeHi = sat(dark ? 60.0 : 50.0); // Light / Midlight
    const int satShadeLo = sat(dark ? 90.0 : 70.0); // Mid / Dark

    // Per-role lightness
    // Nudge lightness slightly as saturation rises to compensate for the
    // Helmholtz-Kohlrausch effect (saturated colors look lighter/heavier).
    const int winL = dark ? (28 + qRound(t * 8)) : (242 - qRound(t * 6));
    const int baseL = dark ? (43 + qRound(t * 6)) : 252;
    const int altL = dark ? (36 + qRound(t * 9)) : (234 - qRound(t * 5));
    const int btnL = dark ? (56 + qRound(t * 10)) : (230 - qRound(t * 10));
    const int tipL = dark ? (52 + qRound(t * 8)) : (248 - qRound(t * 6));

    // Highlight color
    QColor hl;
    if (achromatic) {
        hl = dark ? QColor(105, 105, 105) : QColor(95, 95, 95);
    } else if (dark) {
        int L = qBound(105, accent.lightness() + qRound(45.0 * (1.0 - t)), 215);
        hl = QColor::fromHsl(h, qMin(255, satHighlight), L);
    } else {
        int L = qBound(50, accent.lightness() - qRound(25.0 * t), 180);
        hl = QColor::fromHsl(h, qMin(255, satHighlight), L);
    }
    const double hlLuma = 0.299 * hl.red() + 0.587 * hl.green() + 0.114 * hl.blue();
    const QColor hlText = (hlLuma > 135) ? Qt::black : Qt::white;

    // Local helpers
    using CR = QPalette::ColorRole;
    using CG = QPalette::ColorGroup;

    auto hsl = [&](int lightness, int s) -> QColor {
        return QColor::fromHsl(h, qBound(0, s, 255), qBound(0, lightness, 255));
    };

    auto textOn = [](const QColor &bg) -> QColor {
        double luma = 0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue();
        return (luma > 135) ? Qt::black : Qt::white;
    };

    auto set3 = [&](CR role, QColor active, QColor disabled, QColor inactive) {
        cfg.colors[CG::Active][role] = active;
        cfg.colors[CG::Disabled][role] = disabled;
        cfg.colors[CG::Inactive][role] = inactive;
    };

    auto setAll = [&](CR role, QColor c) { set3(role, c, c, c); };

    // Tooltip: blend classic yellow → hue-tinted above t≈0.20
    QColor bg_tip;
    if (achromatic || t < 0.20) {
        bg_tip = QColor(255, 255, 220);
    } else {
        QColor tinted = hsl(tipL, satTooltip);
        double blend = qMin(1.0, (t - 0.20) / 0.55);
        QColor yellow(255, 255, 220);
        bg_tip = QColor(qRound(yellow.red() * (1.0 - blend) + tinted.red() * blend),
                        qRound(yellow.green() * (1.0 - blend) + tinted.green() * blend),
                        qRound(yellow.blue() * (1.0 - blend) + tinted.blue() * blend));
    }

    // Backgrounds
    const QColor bg_win = hsl(winL, satWindow);
    const QColor bg_base = hsl(baseL, satBase);
    const QColor bg_alt = hsl(altL, satAlt);
    const QColor bg_btn = hsl(btnL, satButton);

    set3(CR::Window, bg_win, bg_win, bg_win);
    set3(CR::Base, bg_base, bg_win, bg_base);
    set3(CR::AlternateBase, bg_alt, bg_alt, bg_alt);
    set3(CR::Button, bg_btn, bg_win, bg_btn);
    set3(CR::ToolTipBase, bg_tip, bg_tip, bg_tip);

    // Foreground text
    const QColor winText = dark ? Qt::white : Qt::black;
    const QColor disText = dark ? QColor(157, 157, 157) : QColor(120, 120, 120);
    const QColor disBtnText = dark ? QColor(120, 120, 120) : QColor(150, 150, 150);

    set3(CR::WindowText, winText, disText, winText);
    set3(CR::Text, winText, disText, winText);
    set3(CR::ButtonText, textOn(bg_btn), disBtnText, textOn(bg_btn));
    setAll(CR::ToolTipText, textOn(bg_tip));

    const QColor phAlpha = dark ? QColor(255, 255, 255, 110) : QColor(0, 0, 0, 110);
    const QColor phDis = dark ? QColor(255, 255, 255, 70) : QColor(0, 0, 0, 70);
    set3(CR::PlaceholderText, phAlpha, phDis, phAlpha);

    // Highlight / selection
    const QColor inactiveHl = hsl(winL + (dark ? 14 : -10), satWindow);
    cfg.colors[CG::Active][CR::Highlight] = hl;
    cfg.colors[CG::Disabled][CR::Highlight] = inactiveHl;
    cfg.colors[CG::Inactive][CR::Highlight] = inactiveHl;
    cfg.colors[CG::Active][CR::HighlightedText] = hlText;
    cfg.colors[CG::Disabled][CR::HighlightedText] = disText;
    cfg.colors[CG::Inactive][CR::HighlightedText] = dark ? Qt::white : Qt::black;

    // BrightText
    QColor bright;
    if (achromatic) {
        bright = dark ? Qt::white : Qt::black;
    } else if (dark) {
        bright = QColor::fromHsl(h, qMin(255, satHighlight + 25), qMin(235, hl.lightness() + 50));
    } else {
        bright = Qt::white;
    }
    setAll(CR::BrightText, bright);

    // Links
    QColor link, linkV;
    if (achromatic) {
        link = dark ? QColor(100, 200, 255) : QColor(0, 0, 210);
        linkV = dark ? QColor(200, 100, 255) : QColor(128, 0, 180);
    } else if (dark) {
        link = QColor::fromHsl(h, qMin(255, satHighlight), qMin(230, hl.lightness() + 75));
        linkV = QColor::fromHsl((h + 30) % 360, qMin(255, satHighlight), qMin(215, hl.lightness() + 55));
    } else {
        link = QColor::fromHsl(h, qMin(255, satHighlight), qMax(40, hl.lightness() - 75));
        linkV = QColor::fromHsl((h + 30) % 360, qMin(255, satHighlight), qMax(30, hl.lightness() - 95));
    }
    set3(CR::Link, link, dark ? QColor(48, 140, 198) : QColor(0, 0, 255), link);
    set3(CR::LinkVisited, linkV, dark ? QColor(180, 80, 255) : QColor(255, 0, 255), linkV);

    // 3D / frame shading
    if (dark) {
        setAll(CR::Light, hsl(115, qMin(255, satShadeHi)));
        setAll(CR::Midlight, hsl(82, qMin(255, satShadeHi)));
        setAll(CR::Mid, hsl(37, satShadeLo));
        setAll(CR::Dark, hsl(22, satShadeLo));
        setAll(CR::Shadow, Qt::black);
    } else {
        setAll(CR::Light, Qt::white);
        setAll(CR::Midlight, hsl(226, qMin(255, satShadeHi)));
        setAll(CR::Mid, hsl(158, satShadeLo));
        setAll(CR::Dark, hsl(148, satShadeLo));
        // Shadow stays neutral — tinting it makes the UI look bruised
        cfg.colors[CG::Active][CR::Shadow] = QColor(105, 105, 105);
        cfg.colors[CG::Disabled][CR::Shadow] = Qt::black;
        cfg.colors[CG::Inactive][CR::Shadow] = QColor(105, 105, 105);
    }

    return cfg;
}

} // namespace PaletteGenerator