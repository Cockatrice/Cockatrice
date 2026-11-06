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
 * Style is a treatment, not a color: the tint still decides the tone, and the
 * style decides how much shape carries that tone.
 *
 * Line is the default and the behavior every caller had before the axis
 * existed: the recolored glyph alone, at the author's stroke weight. It stays
 * the right choice for navigation, menu and toolbar glyphs, where a row of
 * badges would outshout the content.
 *
 * Badge reproduces the pre-#7595 ready/not-ready art: a filled disc, a dark ring
 * and a contrast glyph derived from the tone. It costs attention as a filled
 * shape, so it is opt-in per call site and it is not a generic semantic
 * treatment: the add/remove/delete icons were filled glyphs with an outline and
 * no disc, so badging them would invent a look the UI never had. Authored art
 * that carries its own colors is never badged; it falls back to the asset path.
 * Disabled icons also fall back to the line glyph, because a filled disc at full
 * mass does not read as unavailable.
 *
 * Sticker reproduces the other half of the pre-#7595 set: the filled add, remove
 * and delete glyphs, painted with a tone gradient and a dark outline and no
 * disc. It shares the disabled fallback for the same reason.
 */
enum class IconStyle
{
    Line,    // the recolored glyph alone
    Badge,   // filled disc + dark ring + contrast glyph
    Sticker, // filled glyph + dark outline, no disc
};

/**
 * @brief Selects the color that replaces the `currentColor` sentinel.
 *
 * A palette role follows Qt's structural palette. An application color carries
 * meaning that QPalette cannot express, such as positive and negative states.
 * Callers pick the role by intent; the theme owns the concrete value.
 */
class IconTint
{
public:
    IconTint() = default;
    IconTint(QPalette::ColorRole role); // implicit: existing palette-role callers
    IconTint(AppColor::Role role);      // implicit: semantic callers

    /**
     * @brief Resolves the concrete color for an icon mode.
     *
     * Disabled application colors resolve to the disabled foreground so an
     * unavailable control does not keep signalling its semantic color. Selected
     * palette colors keep the existing WindowText -> HighlightedText swap.
     * Application colors pass through the filled treatments' saturation floor,
     * so a line glyph, a badge disc and a sticker fill carry the same tone
     * instead of a pale line beside a saturated shape.
     *
     * The neutral label roles (WindowText, Text) resolve to icon ink rather than
     * to the raw palette color: theme_icon_ink gives the glyph a share of the
     * accent chroma and holds it to a contrast floor, because a 2px glyph
     * painted in body-text ink reads thinner than the text beside it. Semantic
     * roles keep the hue the call site asked for.
     */
    QColor resolve(QIcon::Mode mode) const;

private:
    enum class Source
    {
        Palette,
        AppColor,
    };

    Source source = Source::Palette;
    QPalette::ColorRole paletteRole = QPalette::WindowText;
    AppColor::Role appRole = AppColor::AccentStrong;
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
 * @param style Treatment of the glyph: a line, or one of the filled treatments
 *              that keep the pre-migration art reading. Defaults to Line.
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
 * @param style Treatment of the glyph: a line, or one of the filled treatments.
 *              A treatment on a non-sentinel asset stays null, like the line
 *              path.
 * @return The recolored pixmap, or a null pixmap when the asset is not a
 *         sentinel-carrying SVG.
 */
QPixmap loadSentinelSvgPixmap(const QString &resolvedPath,
                              IconTint tint = {},
                              const QSize &size = {},
                              IconStyle style = IconStyle::Line);

#endif // THEME_ICON_H
