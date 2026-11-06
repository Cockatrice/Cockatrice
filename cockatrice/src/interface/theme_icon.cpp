#include "theme_icon.h"

#include "libcockatrice/utility/svg_recolor.h"
#include "theme_icon_badge.h"
#include "theme_icon_sticker.h"
#include "theme_manager.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QIconEngine>
#include <QImageReader>
#include <QObject>
#include <QPaintDevice>
#include <QPainter>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QStringLiteral>
#include <QStyle>
#include <QSvgRenderer>
#include <QtGlobal>
#include <utility>

IconTint::IconTint(QPalette::ColorRole role) : paletteRole(role)
{
}

IconTint::IconTint(AppColor::Role role) : source(Source::AppColor), appRole(role)
{
}

QColor IconTint::resolve(QIcon::Mode mode) const
{
    if (source == Source::AppColor) {
        // A disabled control must not keep signalling its semantic color.
        if (mode == QIcon::Disabled) {
            return qApp->palette().color(QPalette::Disabled, QPalette::WindowText);
        }
        const QColor color = themeManager->appColor(appRole);
        if (mode != QIcon::Selected) {
            return color;
        }
        // Selected rows paint their icon on the highlight colour. Keep the
        // semantic tint when it stays legible there; otherwise the glyph would
        // vanish into the selection.
        constexpr double MIN_SELECTED_CONTRAST = 3.0;
        const QColor highlight = qApp->palette().color(QPalette::Active, QPalette::Highlight);
        return ThemeIconBadge::contrastRatio(color, highlight) >= MIN_SELECTED_CONTRAST
                   ? color
                   : qApp->palette().color(QPalette::Active, QPalette::HighlightedText);
    }

    const QPalette::ColorGroup group = mode == QIcon::Disabled ? QPalette::Disabled : QPalette::Active;
    const QPalette::ColorRole effective =
        mode == QIcon::Selected && paletteRole == QPalette::WindowText ? QPalette::HighlightedText : paletteRole;
    return qApp->palette().color(group, effective);
}

namespace
{
/**
 * @brief Reads an SVG through the `theme:` search path.
 */
QByteArray readSvg(const QString &resolvedPath)
{
    QFile file(QStringLiteral("theme:") + resolvedPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

/**
 * @brief Renders SVG bytes, scaled to @p size when one is given.
 *
 * An empty size means the asset's intrinsic size; @p devicePixelRatio only
 * applies in that case so HiDPI callers still get a crisp glyph.
 */
QPixmap renderSvg(const QByteArray &data, const QSize &size, qreal devicePixelRatio)
{
    QSvgRenderer renderer(data);
    if (!renderer.isValid()) {
        return {};
    }

    QSize renderSize = size;
    if (renderSize.isEmpty()) {
        renderSize = renderer.defaultSize();
        renderSize =
            QSize(qRound(renderSize.width() * devicePixelRatio), qRound(renderSize.height() * devicePixelRatio));
    }
    if (!renderSize.isValid() || renderSize.isEmpty()) {
        return {};
    }

    QPixmap pixmap(renderSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return pixmap;
}

bool hasSentinel(const QByteArray &svg)
{
    return !svg.isEmpty() && QString::fromUtf8(svg).contains(QStringLiteral("currentColor"), Qt::CaseInsensitive);
}

QString cacheKey(const QString &path, const QByteArray &variant, const QSize &size, qreal devicePixelRatio)
{
    return path + QLatin1Char('|') + QString::fromLatin1(variant) + QLatin1Char('|') + QString::number(size.width()) +
           QLatin1Char('x') + QString::number(size.height()) + QLatin1Char('@') + QString::number(devicePixelRatio);
}

/**
 * @brief QIcon engine rendering a `theme:` SVG with `currentColor` resolved on demand.
 *
 * Qt's SVG renderer has no ambient color, so the sentinel is substituted from
 * the current palette on every pixmap request. Asset resolution, the file bytes
 * and the intrinsic size are cached until the theme changes: a repaint then
 * does no file I/O at all. The render cache is keyed by resolved path and pixel
 * size plus a variant: the palette color for sentinel SVGs, the icon mode for
 * assets that carry their own colors, and the tone plus glyph color for a badge.
 * A palette change therefore produces new keys instead of reusing art built
 * under the previous palette, and the non-sentinel path (raster overrides, plain
 * SVGs) is cached like any other.
 *
 * The engine only resolves and caches. The badge shape itself is painted by
 * ThemeIconBadge so the compositing stays out of the resolution path.
 */
class ThemeSvgIconEngine final : public QObject, public QIconEngine
{
public:
    ThemeSvgIconEngine(QString _prefix, IconTint _tint, IconStyle _style)
        : prefix(std::move(_prefix)), tint(_tint), style(_style)
    {
        if (themeManager) {
            // A theme switch changes the resolved asset (scheme-qualified
            // variants, raster overrides) and a palette change invalidates
            // every cached colour, so both drop the caches instead of
            // re-reading theme.cfg and the assets on every repaint.
            connect(themeManager, &ThemeManager::themeChanged, this, &ThemeSvgIconEngine::invalidate);
            connect(themeManager, &ThemeManager::paletteChanged, this, &ThemeSvgIconEngine::invalidate);
        }
    }

    QIconEngine *clone() const override
    {
        return new ThemeSvgIconEngine(prefix, tint, style);
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State) override
    {
        const qreal devicePixelRatio = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
        const QSize logicalSize = fitToSize(rect.size());
        if (logicalSize.isEmpty()) {
            return;
        }
        const QSize pixelSize(qRound(logicalSize.width() * devicePixelRatio),
                              qRound(logicalSize.height() * devicePixelRatio));
        const QPixmap pixmap = render(pixelSize, devicePixelRatio, mode);
        if (!pixmap.isNull()) {
            // QRect::center() would floor and drop the last row/column of a
            // 16x16 glyph; the center has to be fractional so glyphs stay
            // unshifted and unsclipped in item views.
            const QSizeF contentSize = pixmap.deviceIndependentSize();
            const QRectF targetRect(
                QRectF(rect).center() - QPointF(contentSize.width() / 2.0, contentSize.height() / 2.0), contentSize);
            painter->drawPixmap(targetRect, pixmap, QRectF(pixmap.rect()));
        }
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State) override
    {
        return render(fitToSize(size), 1.0, mode);
    }

    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State, qreal scale) override
    {
        const QSize logicalSize = fitToSize(size);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        const QSize pixelSize(qRound(logicalSize.width() * scale), qRound(logicalSize.height() * scale));
#else
        // Qt < 6.8 passes the device-dependent size to the engine.
        const QSize pixelSize = logicalSize;
#endif
        return render(pixelSize, scale, mode);
    }

    QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override
    {
        return fitToSize(size);
    }

private:
    void invalidate()
    {
        hasResolvedAsset = false;
        hasIntrinsicSize = false;
        cache.clear();
    }

    void ensureResolved() const
    {
        if (hasResolvedAsset && (!themeManager || themeManager->getCurrentThemePath() == resolvedThemePath)) {
            return;
        }
        resolvedThemePath = themeManager ? themeManager->getCurrentThemePath() : QString();
        resolvedPath = themeManager ? themeManager->assetPath(prefix) : QString();
        resolvedAssetIsSvg = resolvedPath.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive);
        resolvedSvg = resolvedAssetIsSvg ? readSvg(resolvedPath) : QByteArray();
        resolvedHasSentinel = hasSentinel(resolvedSvg);
        hasResolvedAsset = true;
        hasIntrinsicSize = false;
    }

    QSize intrinsicSize() const
    {
        ensureResolved();
        if (!hasIntrinsicSize) {
            if (resolvedAssetIsSvg) {
                QSvgRenderer renderer(resolvedSvg);
                intrinsicSizeCache = renderer.isValid() ? renderer.defaultSize() : QSize();
            } else {
                // A theme may override a glyph with a raster; take its real
                // aspect ratio instead of assuming the 24x24 SVG default, so
                // actualSize() and the rendered pixmap agree.
                QImageReader reader(QStringLiteral("theme:") + resolvedPath);
                intrinsicSizeCache = reader.size();
            }
            if (intrinsicSizeCache.isEmpty()) {
                intrinsicSizeCache = QSize(24, 24);
            }
            hasIntrinsicSize = true;
        }
        return intrinsicSizeCache;
    }

    QSize fitToSize(const QSize &size) const
    {
        if (size.isEmpty()) {
            return size;
        }
        return intrinsicSize().scaled(size, Qt::KeepAspectRatio);
    }

    QPixmap fromAsset(const QSize &pixelSize, QIcon::Mode mode) const
    {
        QPixmap pixmap(QStringLiteral("theme:") + resolvedPath);
        if (!pixelSize.isEmpty() && pixmap.size() != pixelSize) {
            pixmap = pixmap.scaled(pixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        if (mode == QIcon::Disabled && !pixmap.isNull()) {
            pixmap = QApplication::style()->generatedIconPixmap(QIcon::Disabled, pixmap, nullptr);
        }
        return pixmap;
    }

    QPixmap render(const QSize &pixelSize, qreal devicePixelRatio, QIcon::Mode mode) const
    {
        ensureResolved();

        // Badge and Sticker are shape treatments, so they only apply to sentinel
        // art: an authored asset keeps its own drawing and follows the line/asset
        // path. Disabled also drops back to the line glyph: a filled disc or
        // sticker at full mass does not read as unavailable next to the thin line
        // glyphs around it, and the tint already removes the semantic tone.
        const bool treated = style != IconStyle::Line && resolvedHasSentinel && mode != QIcon::Disabled;
        const bool badge = treated && style == IconStyle::Badge;
        const bool sticker = treated && style == IconStyle::Sticker;
        // Same cache for all paths: sentinel art varies by palette color, art
        // with its own colors varies by mode (Disabled is generated). A color
        // variant is never a bare digit, so the key spaces cannot collide. A
        // badge also varies by glyph color, because the same tone can switch
        // between a white and a near-black glyph at different lightnesses.
        const QColor tone = resolvedHasSentinel ? tint.resolve(mode) : QColor();
        QByteArray variant;
        if (badge) {
            variant = QByteArrayLiteral("B|") + tone.name(QColor::HexRgb).toLatin1() + '|' +
                      ThemeIconBadge::glyphColor(tone).name(QColor::HexRgb).toLatin1();
        } else if (sticker) {
            variant = QByteArrayLiteral("S|") + tone.name(QColor::HexRgb).toLatin1();
        } else if (resolvedHasSentinel) {
            variant = tone.name(QColor::HexRgb).toLatin1();
        } else {
            variant = QByteArray::number(static_cast<int>(mode));
        }
        const QString key = cacheKey(resolvedPath, variant, pixelSize, devicePixelRatio);
        const auto cached = cache.constFind(key);
        if (cached != cache.constEnd()) {
            return cached.value();
        }

        QPixmap pixmap;
        if (badge) {
            // Geometry is computed from the device-pixel size, and the ratio is
            // attached below like every other path.
            pixmap = ThemeIconBadge::renderBadge(resolvedSvg, tone, pixelSize);
        } else if (sticker) {
            pixmap = ThemeIconSticker::renderSticker(resolvedSvg, tone, pixelSize);
        } else if (resolvedHasSentinel) {
            pixmap = renderSvg(resolveSvgColorSentinel(resolvedSvg, variant), pixelSize, devicePixelRatio);
        } else {
            // Authored art carries its own colors: use Qt's normal loader so
            // raster overrides and non-SVG assets keep working.
            pixmap = fromAsset(pixelSize, mode);
        }
        if (!pixmap.isNull()) {
            // Set the ratio before caching: assigning it to a cached pixmap
            // afterwards detaches (deep-copies) it on every paint.
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
        if (cache.size() >= 64) {
            cache.clear();
        }
        cache.insert(key, pixmap);
        return pixmap;
    }

    QString prefix;
    IconTint tint;
    IconStyle style;
    mutable QHash<QString, QPixmap> cache;
    mutable QString resolvedPath;
    mutable QString resolvedThemePath;
    mutable QByteArray resolvedSvg;
    mutable QSize intrinsicSizeCache;
    mutable bool resolvedAssetIsSvg = false;
    mutable bool resolvedHasSentinel = false;
    mutable bool hasResolvedAsset = false;
    mutable bool hasIntrinsicSize = false;
};
} // namespace

QIcon loadThemedIcon(QStringView prefix, IconTint tint, IconStyle style)
{
    return QIcon(new ThemeSvgIconEngine(prefix.toString(), tint, style));
}

QPixmap loadSentinelSvgPixmap(const QString &resolvedPath, IconTint tint, const QSize &size, IconStyle style)
{
    if (!resolvedPath.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)) {
        return {};
    }

    const QByteArray svg = readSvg(resolvedPath);
    if (!hasSentinel(svg)) {
        return {};
    }

    const QColor tone = tint.resolve(QIcon::Normal);
    if (style == IconStyle::Badge || style == IconStyle::Sticker) {
        QSize canvas = size;
        if (canvas.isEmpty()) {
            canvas = QSvgRenderer(svg).defaultSize();
        }
        return style == IconStyle::Badge ? ThemeIconBadge::renderBadge(svg, tone, canvas)
                                         : ThemeIconSticker::renderSticker(svg, tone, canvas);
    }

    return renderSvg(resolveSvgColorSentinel(svg, tone.name(QColor::HexRgb).toLatin1()), size, 1.0);
}
