#include "theme_icon_badge.h"

#include "libcockatrice/utility/svg_recolor.h"
#include "theme_icon_fill.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QSvgRenderer>
#include <QtGlobal>

namespace
{
// The pre-#7595 badge art filled almost the whole canvas: a 24px icon carried a
// 23px disc with a hairline black outline. Keep the disc large and the ring
// thin so the glyph still has room at 16px.
constexpr double RING_WIDTH_RATIO = 0.04;
// The sentinel paths sit inside a padded 24-unit viewBox (the check spans 4..20),
// so the glyph box has to be nearly the whole disc for the engraved mark to
// match the old art's weight. At half the disc it read as a smudge at 15px.
constexpr double GLYPH_DIAMETER_RATIO = 0.9;
// The sentinel set is drawn at 2 units in a 24-unit viewBox, which turns into a
// hairline once the glyph is scaled into a 15px disc. The old badge carried a
// band near a tenth of the disc diameter, so the badge decides the glyph weight
// instead of inheriting the authored one.
constexpr qreal GLYPH_STROKE_WIDTH = 3.0;
constexpr int RING_DARKER = 300;
constexpr int GLYPH_DARKER = 400;
} // namespace

namespace
{
struct DiscFill
{
    QColor top;
    QColor bottom;
};

DiscFill discFill(const QColor &tone)
{
    return {ThemeIconFill::topColor(tone), ThemeIconFill::bottomColor(tone)};
}
} // namespace

namespace ThemeIconBadge
{
QColor glyphColor(const QColor &tone, ThemeIconFill::Scheme scheme)
{
    // The badge carries the ink the pre-sticker badge did: white on the light
    // scheme, and the tone darkened to an outline-dark ink on the dark scheme.
    // The lifted disc is light enough there that white sits near 2.7:1 at the
    // top, so the dark ink is the deliberate choice rather than a contrast
    // measurement that a palette tweak could flip. Deriving it from the tone
    // keeps themes in control, and the scheme is explicit because an authored
    // quiet level is a dark tone on the dark scheme too.
    return scheme == ThemeIconFill::Scheme::Dark ? tone.darker(GLYPH_DARKER) : QColor(Qt::white);
}

QSize glyphSize(const QSize &pixelSize)
{
    if (pixelSize.isEmpty()) {
        return {};
    }
    const qreal side = qMin(pixelSize.width(), pixelSize.height());
    const qreal ringWidth = qMax(1.0, side * RING_WIDTH_RATIO);
    const qreal discDiameter = side - 2.0 * ringWidth;
    const int glyphSide = qMax(1, qRound(discDiameter * GLYPH_DIAMETER_RATIO));
    return QSize(glyphSide, glyphSide);
}

QPixmap renderBadge(const QByteArray &svg, const QColor &tone, const QSize &pixelSize, ThemeIconFill::Scheme scheme)
{
    if (svg.isEmpty() || !tone.isValid() || pixelSize.isEmpty()) {
        return {};
    }

    QPixmap pixmap(pixelSize);
    pixmap.fill(Qt::transparent);

    const qreal side = qMin(pixelSize.width(), pixelSize.height());
    const qreal ringWidth = qMax(1.0, side * RING_WIDTH_RATIO);
    const qreal discDiameter = side - 2.0 * ringWidth;

    QRectF discRect(0, 0, discDiameter, discDiameter);
    discRect.moveCenter(QRectF(QPointF(0, 0), QSizeF(pixelSize)).center());

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient gradient(discRect.topLeft(), discRect.bottomLeft());
    const DiscFill fill = discFill(tone);
    gradient.setColorAt(0.0, fill.top);
    gradient.setColorAt(1.0, fill.bottom);
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawEllipse(discRect);

    // The ring closes the disc: its outer edge touches the canvas, its inner
    // edge is the disc boundary, so the two meet without a seam.
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(tone.darker(RING_DARKER), ringWidth));
    const qreal ringRadius = (side - ringWidth) / 2.0;
    painter.drawEllipse(discRect.center(), ringRadius, ringRadius);

    const QByteArray glyphSvg = resolveSvgStrokeWidth(
        resolveSvgColorSentinel(svg, glyphColor(tone, scheme).name(QColor::HexRgb).toLatin1()), GLYPH_STROKE_WIDTH);
    QSvgRenderer renderer(glyphSvg);
    if (renderer.isValid()) {
        const QSize size = glyphSize(pixelSize);
        const QRectF glyphRect(
            QPointF(discRect.center().x() - size.width() / 2.0, discRect.center().y() - size.height() / 2.0),
            QSizeF(size));
        renderer.render(&painter, glyphRect);
    }

    return pixmap;
}
} // namespace ThemeIconBadge
