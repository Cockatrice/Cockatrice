/**
 * @file theme_icon.h
 * @ingroup Interface
 */

#ifndef THEME_ICON_H
#define THEME_ICON_H

#include <QIcon>
#include <QPalette>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringView>

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
 * @param role Palette role whose color replaces the sentinel. Selected glyphs
 *             use HighlightedText unless a role other than WindowText is given.
 * @return An icon that follows the application palette.
 */
QIcon loadThemedIcon(QStringView prefix, QPalette::ColorRole role = QPalette::WindowText);

/**
 * @brief Renders a resolved theme SVG when it carries the `currentColor` sentinel.
 *
 * Non-SVG assets and SVGs without the sentinel return a null pixmap so callers
 * can fall back to their usual loading path (including legacy inversion).
 *
 * @param resolvedPath Asset path relative to the `theme:` search path, as
 *                     returned by ThemeManager::assetPath (includes extension).
 * @param role Palette role whose color replaces the sentinel.
 * @param size Pixel size to render at; an empty size uses the SVG's intrinsic size.
 * @return The recolored pixmap, or a null pixmap when the asset is not a
 *         sentinel-carrying SVG.
 */
QPixmap loadSentinelSvgPixmap(const QString &resolvedPath,
                              QPalette::ColorRole role = QPalette::WindowText,
                              const QSize &size = {});

#endif // THEME_ICON_H
