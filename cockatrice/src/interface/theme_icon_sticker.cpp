#include "theme_icon_sticker.h"

#include "libcockatrice/utility/svg_recolor.h"
#include "theme_icon_fill.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSvgRenderer>
#include <QtGlobal>

namespace
{
// The pre-#7595 add/remove art filled about 80% of the canvas: the sentinel
// paths are padded inside their 24-unit viewBox, so the drawing box has to be
// slightly larger than the canvas for the ink to reach that far.
constexpr qreal STICKER_SCALE = 1.06;
// Old arm thickness was about a fifth of the canvas, with a rim near 5%. In the
// 24-unit viewBox that is a 4.5-unit band with a 0.9-unit rim on each side.
constexpr qreal STICKER_STROKE_WIDTH = 4.5;
constexpr qreal STICKER_OUTLINE_WIDTH = 0.9;
// The old art's outline was black. Darkening by 300 reads as black on a light
// scheme (the shipped tones are already dark there) while staying a visible dark
// rim on a dark one, where a neutral black would vanish into the window.
constexpr int OUTLINE_DARKER = 300;
} // namespace

namespace ThemeIconSticker
{
QColor outlineColor(const QColor &tone)
{
    return ThemeIconFill::saturated(tone).darker(OUTLINE_DARKER);
}

QPixmap renderSticker(const QByteArray &svg, const QColor &tone, const QSize &pixelSize)
{
    if (svg.isEmpty() || !tone.isValid() || pixelSize.isEmpty()) {
        return {};
    }

    QPixmap pixmap(pixelSize);
    pixmap.fill(Qt::transparent);

    const qreal side = qMin(pixelSize.width(), pixelSize.height());
    const qreal box = side * STICKER_SCALE;
    const QRectF glyphRect(QPointF((pixelSize.width() - box) / 2.0, (pixelSize.height() - box) / 2.0),
                           QSizeF(box, box));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Outline first, at the band width plus one rim on each side; the fill pass
    // then covers the middle and leaves only the rim visible.
    const QByteArray outlineSvg =
        resolveSvgStrokeWidth(resolveSvgColorSentinel(svg, outlineColor(tone).name(QColor::HexRgb).toLatin1()),
                              STICKER_STROKE_WIDTH + 2.0 * STICKER_OUTLINE_WIDTH);
    QSvgRenderer outlineRenderer(outlineSvg);
    if (outlineRenderer.isValid()) {
        outlineRenderer.render(&painter, glyphRect);
    }

    // The fill needs its own pass so the gradient can be clipped to the glyph:
    // painting it onto the composited pixmap would tint the outline as well.
    QPixmap fillPixmap(pixelSize);
    fillPixmap.fill(Qt::transparent);
    {
        QPainter fillPainter(&fillPixmap);
        fillPainter.setRenderHint(QPainter::Antialiasing, true);

        const QByteArray fillSvg = resolveSvgStrokeWidth(
            resolveSvgColorSentinel(svg, tone.name(QColor::HexRgb).toLatin1()), STICKER_STROKE_WIDTH);
        QSvgRenderer fillRenderer(fillSvg);
        if (fillRenderer.isValid()) {
            fillRenderer.render(&fillPainter, glyphRect);
            fillPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            QLinearGradient gradient(glyphRect.topLeft(), glyphRect.bottomLeft());
            gradient.setColorAt(0.0, ThemeIconFill::topColor(tone));
            gradient.setColorAt(1.0, ThemeIconFill::bottomColor(tone));
            fillPainter.fillRect(QRectF(QPointF(0, 0), QSizeF(pixelSize)), gradient);
        }
    }
    painter.drawPixmap(0, 0, fillPixmap);

    return pixmap;
}
} // namespace ThemeIconSticker
