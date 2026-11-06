/**
 * @file theme_icon.h
 * @ingroup Interface
 */

#ifndef THEME_ICON_H
#define THEME_ICON_H

#include "theme_config.h"

#include <QColor>
#include <QIcon>
#include <QPalette>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringView>

/**
 * @brief Selects how a `currentColor` glyph is painted.
 *
 * The tint decides the tone. The style decides how much shape carries it.
 *
 * Line is the default and matches what every caller did before this axis
 * existed: the recolored glyph alone, at the author's stroke weight. Use it for
 * navigation, menu and toolbar glyphs, where a row of badges would compete with
 * the content.
 *
 * Badge reproduces the pre-#7595 ready/not-ready art: a filled disc, a dark ring
 * and a contrast glyph derived from the tone. A filled shape draws more
 * attention than a line, so it is opt-in per call site. It was made for the
 * ready/not-ready icons. The add/remove/delete icons were filled glyphs with an
 * outline and no disc, so badging them would produce art the UI never had.
 * Authored art that carries its own colors is never badged and falls back to the
 * asset path. Disabled icons fall back to the line glyph too, because a filled
 * disc at full mass does not read as unavailable.
 *
 * Sticker reproduces the other half of the pre-#7595 set: the filled add, remove
 * and delete glyphs, painted with a tone gradient and a dark outline and no
 * disc. It shares the disabled fallback for the same reason.
 */
enum class IconStyle
{
    Line,    ///< the recolored glyph alone
    Badge,   ///< filled disc + dark ring + contrast glyph
    Sticker, ///< filled glyph + dark outline, no disc
};

/**
 * @brief Selects how much attention a glyph claims inside its surface.
 *
 * Tint says which hue, style says how much shape, emphasis says how loud. The
 * three are independent: a primary action can be a line glyph in the theme
 * accent, and a quiet one can be a sticker in a dimmed tone.
 *
 * Secondary is the default and the color every icon had before the axis
 * existed, so existing call sites keep their look. Primary is for the one
 * action a surface exists for: a neutral primary glyph takes the accent, and a
 * tinted one is pushed to full contrast. Quiet is for glyphs that label rather
 * than act, such as the magnifier inside a search field: legible, but never
 * competing with the buttons next to it.
 */
enum class IconEmphasis
{
    Quiet,     // labels rather than acts
    Secondary, // the default action weight
    Primary,   // the action the surface exists for
};

/**
 * @brief Selects the color that replaces the `currentColor` sentinel.
 *
 * A palette role follows Qt's structural palette. An application color carries
 * meaning that QPalette cannot express, such as positive and negative states.
 * Callers pick the role by intent; the theme owns the concrete value. Emphasis
 * is part of the same decision: both answer how the glyph is inked.
 */
class IconTint
{
public:
    IconTint() = default;
    IconTint(QPalette::ColorRole role, IconEmphasis emphasis = IconEmphasis::Secondary); // implicit for role callers
    IconTint(AppColor::Role role, IconEmphasis emphasis = IconEmphasis::Secondary); // implicit for semantic callers

    /** @brief A neutral glyph in the accent tone, for the surface's primary action. */
    static IconTint primary();

    /** @brief A dimmed neutral glyph, for a control that labels rather than acts. */
    static IconTint quiet();

    /**
     * @brief Resolves the concrete color for an icon mode.
     *
     * A disabled application color resolves to the disabled foreground, so an
     * unavailable control does not keep its semantic color. A selected palette
     * color keeps the WindowText -> HighlightedText swap. Application colors go
     * through the saturation floor shared by the filled treatments, so a line
     * glyph, a badge disc and a sticker fill carry the same tone.
     *
     * Neutral label roles (WindowText, Text) resolve to icon ink rather than the
     * raw palette color: ThemeIconInk::glyphInk borrows a little accent chroma
     * and holds the glyph to a contrast floor, because a 2px glyph in body-text
     * ink reads thinner than the text beside it. Semantic roles keep the hue the
     * call site asked for.
     *
     * Emphasis is applied after the hue: Primary lifts a neutral glyph to the
     * accent and pushes a tinted one to full contrast; Quiet blends either
     * toward the surface down to the non-text legibility floor.
     */
    QColor resolve(QIcon::Mode mode) const;

    IconEmphasis getEmphasis() const
    {
        return emphasis;
    }

private:
    enum class Source
    {
        Palette,
        AppColor,
    };

    Source source = Source::Palette;
    QPalette::ColorRole paletteRole = QPalette::WindowText;
    AppColor::Role appRole = AppColor::AccentStrong;
    IconEmphasis emphasis = IconEmphasis::Secondary;
};

/**
 * @brief Builds a palette-aware icon from a `theme:` asset.
 *
 * SVG glyphs may use the `currentColor` sentinel. A plain QPixmap snapshot
 * cannot follow palette changes, so this returns a QIcon whose engine resolves
 * the sentinel from the current palette every time it is asked for a pixmap.
 * Icons therefore stay legible when the user switches between light and dark
 * themes without recreating any actions or buttons.
 *
 * @param prefix Asset prefix relative to the `theme:` search path, with no file
 *               extension (for example `icons/search`), matching themePixmap().
 * @param tint Color that replaces the sentinel: a palette role or a semantic
 *             application color. Defaults to WindowText.
 * @param style Treatment of the glyph: Line, Badge or Sticker. Defaults to Line.
 * @return An icon that follows the application palette.
 */
QIcon loadThemedIcon(QStringView prefix, IconTint tint = {}, IconStyle style = IconStyle::Line);

/**
 * @brief Renders a resolved theme SVG when it carries the `currentColor` sentinel.
 *
 * Non-SVG assets and SVGs without the sentinel return a null pixmap so callers
 * can fall back to their usual loading path (including legacy inversion).
 *
 * @param resolvedPath Asset path relative to the `theme:` search path, as
 *                     returned by ThemeManager::assetPath (includes extension).
 * @param tint Color that replaces the sentinel: a palette role or a semantic
 *             application color. Defaults to WindowText.
 * @param size Pixel size to render at; an empty size uses the SVG's intrinsic size.
 * @param style Treatment of the glyph: Line, Badge or Sticker. A treatment on a
 *              non-sentinel asset stays null, like the line path.
 * @return The recolored pixmap, or a null pixmap when the asset is not a
 *         sentinel-carrying SVG.
 */
QPixmap loadSentinelSvgPixmap(const QString &resolvedPath,
                              IconTint tint = {},
                              const QSize &size = {},
                              IconStyle style = IconStyle::Line);

#endif // THEME_ICON_H
