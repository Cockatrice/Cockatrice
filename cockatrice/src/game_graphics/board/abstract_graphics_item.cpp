#include "abstract_graphics_item.h"

#include <QPainter>

void AbstractGraphicsItem::paintNumberEllipse(int number,
                                              int fontSize,
                                              const QColor &color,
                                              int position,
                                              int count,
                                              QPainter *painter)
{
    painter->save();

    QString numStr = QString::number(number);
    QFont font("Serif");
    font.setPixelSize(fontSize);
    font.setWeight(QFont::Bold);

    QFontMetrics fm(font);
    double w = 1.3 *
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
               fm.horizontalAdvance(numStr);
#else
               fm.width(numStr);
#endif
    double h = fm.height() * 1.3;
    if (w < h)
        w = h;

    painter->setPen(QColor(255, 255, 255, 0));
    painter->setBrush(QBrush(QColor(color)));

    QRectF textRect;
    if (position == -1)
        textRect = QRectF((boundingRect().width() - w) / 2.0, (boundingRect().height() - h) / 2.0, w, h);
    else {
        qreal xOffset = 10;
        qreal yOffset = 20;
        qreal spacing = 2;
        if (position < 2)
            textRect = QRectF(count == 1 ? ((boundingRect().width() - w) / 2.0)
                                         : (position % 2 == 0 ? xOffset : (boundingRect().width() - xOffset - w)),
                              yOffset, w, h);
        else
            textRect = QRectF(count == 3 ? ((boundingRect().width() - w) / 2.0)
                                         : (position % 2 == 0 ? xOffset : (boundingRect().width() - xOffset - w)),
                              yOffset + (spacing + h) * (position / 2), w, h);
    }

    painter->drawEllipse(textRect);

    painter->setPen(Qt::black);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignCenter, numStr);

    painter->restore();
}

int resetPainterTransform(QPainter *painter)
{
    painter->resetTransform();
    auto tx = painter->deviceTransform().inverted();
    painter->setTransform(tx);
    return tx.isScaling() ? 1.0 / tx.m11() : 1;
}
