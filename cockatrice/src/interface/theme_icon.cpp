#include "theme_icon.h"

#include "libcockatrice/utility/svg_recolor.h"
#include "theme_manager.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QIconEngine>
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

namespace
{
/**
 * @brief Palette color that replaces `currentColor`, following the icon mode.
 */
QByteArray sentinelColor(QIcon::Mode mode, QPalette::ColorRole role)
{
    const QPalette::ColorGroup group = mode == QIcon::Disabled ? QPalette::Disabled : QPalette::Active;
    const QPalette::ColorRole effective =
        mode == QIcon::Selected && role == QPalette::WindowText ? QPalette::HighlightedText : role;
    return qApp->palette().color(group, effective).name(QColor::HexRgb).toLatin1();
}

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

QString cacheKey(const QString &path, const QByteArray &color, const QSize &size, qreal devicePixelRatio)
{
    return path + QLatin1Char('|') + QString::fromLatin1(color) + QLatin1Char('|') + QString::number(size.width()) +
           QLatin1Char('x') + QString::number(size.height()) + QLatin1Char('@') + QString::number(devicePixelRatio);
}

/**
 * @brief QIcon engine rendering a `theme:` SVG with `currentColor` resolved on demand.
 *
 * Qt's SVG renderer has no ambient color, so the sentinel is substituted from
 * the current palette on every pixmap request. The render cache is keyed by
 * resolved path, palette color, pixel size and DPR, so a theme change produces
 * new keys instead of reusing art built under the previous palette.
 */
class ThemeSvgIconEngine final : public QIconEngine
{
public:
    ThemeSvgIconEngine(QString _prefix, QPalette::ColorRole _role) : prefix(std::move(_prefix)), role(_role)
    {
    }

    QIconEngine *clone() const override
    {
        return new ThemeSvgIconEngine(prefix, role);
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
        QPixmap pixmap = render(pixelSize, devicePixelRatio, mode);
        if (!pixmap.isNull()) {
            pixmap.setDevicePixelRatio(devicePixelRatio);
            const QSizeF contentSize = pixmap.deviceIndependentSize();
            const QRectF targetRect(
                QPointF(rect.center()) - QPointF(contentSize.width() / 2.0, contentSize.height() / 2.0), contentSize);
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
        QPixmap pixmap = render(pixelSize, scale, mode);
        if (!pixmap.isNull()) {
            pixmap.setDevicePixelRatio(scale);
        }
        return pixmap;
    }

    QSize actualSize(const QSize &size, QIcon::Mode, QIcon::State) override
    {
        return fitToSize(size);
    }

private:
    void ensureResolved() const
    {
        const QString themePath = themeManager->getCurrentThemePath();
        const bool darkMode = themeManager->isDarkModeActive();
        if (!hasResolvedPath || themePath != resolvedThemePath || darkMode != resolvedDarkMode) {
            resolvedPath = themeManager->assetPath(prefix);
            resolvedThemePath = themePath;
            resolvedDarkMode = darkMode;
            hasResolvedPath = true;
            hasIntrinsicSize = false;
        }
    }

    QSize intrinsicSize() const
    {
        ensureResolved();
        if (!hasIntrinsicSize) {
            const QByteArray svg = readSvg(resolvedPath);
            QSvgRenderer renderer(svg);
            intrinsicSizeCache = renderer.isValid() ? renderer.defaultSize() : QSize(24, 24);
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

    QPixmap render(const QSize &pixelSize, qreal devicePixelRatio, QIcon::Mode mode) const
    {
        ensureResolved();
        const QString themePath = resolvedThemePath;
        const bool darkMode = resolvedDarkMode;

        const QByteArray svg = readSvg(resolvedPath);
        if (svg.isEmpty() || !QString::fromUtf8(svg).contains(QStringLiteral("currentColor"), Qt::CaseInsensitive)) {
            // Authored art carries its own colors: use Qt's normal loader so
            // raster overrides and non-SVG assets keep working.
            QPixmap pixmap(QStringLiteral("theme:") + resolvedPath);
            if (!pixelSize.isEmpty() && pixmap.size() != pixelSize) {
                pixmap = pixmap.scaled(pixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            if (mode == QIcon::Disabled && !pixmap.isNull()) {
                pixmap = QApplication::style()->generatedIconPixmap(QIcon::Disabled, pixmap, nullptr);
            }
            return pixmap;
        }

        const QByteArray color = sentinelColor(mode, role);
        const QString key = cacheKey(resolvedPath, color, pixelSize, devicePixelRatio) + QLatin1Char('|') + themePath +
                            (darkMode ? QLatin1Char('d') : QLatin1Char('l'));
        const auto cached = cache.constFind(key);
        if (cached != cache.constEnd()) {
            return cached.value();
        }

        QPixmap pixmap = renderSvg(resolveSvgColorSentinel(svg, color), pixelSize, devicePixelRatio);
        if (cache.size() >= 64) {
            cache.clear();
        }
        cache.insert(key, pixmap);
        return pixmap;
    }

    QString prefix;
    QPalette::ColorRole role;
    mutable QHash<QString, QPixmap> cache;
    mutable QString resolvedPath;
    mutable QString resolvedThemePath;
    mutable QSize intrinsicSizeCache;
    mutable bool resolvedDarkMode = false;
    mutable bool hasResolvedPath = false;
    mutable bool hasIntrinsicSize = false;
};
} // namespace

QIcon loadThemedIcon(QStringView prefix, QPalette::ColorRole role)
{
    return QIcon(new ThemeSvgIconEngine(prefix.toString(), role));
}

QPixmap loadSentinelSvgPixmap(const QString &resolvedPath, QPalette::ColorRole role, const QSize &size)
{
    if (!resolvedPath.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive)) {
        return {};
    }

    const QByteArray svg = readSvg(resolvedPath);
    if (svg.isEmpty() || !QString::fromUtf8(svg).contains(QStringLiteral("currentColor"), Qt::CaseInsensitive)) {
        return {};
    }

    const QByteArray color = qApp->palette().color(role).name(QColor::HexRgb).toLatin1();
    return renderSvg(resolveSvgColorSentinel(svg, color), size, 1.0);
}
