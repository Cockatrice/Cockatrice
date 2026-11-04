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
 * @return An icon that follows the application palette.
 */
QIcon loadThemedIcon(QStringView prefix, IconTint tint = {});

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
 * @return The recolored pixmap, or a null pixmap when the asset is not a
 *         sentinel-carrying SVG.
 */
QPixmap loadSentinelSvgPixmap(const QString &resolvedPath, IconTint tint = {}, const QSize &size = {});

#endif // THEME_ICON_H
