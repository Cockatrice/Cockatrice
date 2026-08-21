#include "art_crop_attribution.h"

#include <QFontMetrics>
#include <QObject>
#include <QPainter>
#include <libcockatrice/card/printing/exact_card.h>

QString buildArtAttribution(const ExactCard &card)
{
    const QString artist = card.getPrinting().getArtist();
    if (artist.isEmpty()) {
        return QString();
    }
    return QObject::tr("Art: %1").arg(artist);
}

QRectF paintArtAttribution(QPainter &painter,
                           const QRectF &rect,
                           const QString &attribution,
                           Qt::Alignment anchor,
                           qreal scale)
{
    if (attribution.isEmpty()) {
        return QRectF();
    }

    painter.save();

    QFont font = painter.font();
    font.setPointSizeF(qMax(6.0, font.pointSizeF() * scale));
    painter.setFont(font);

    const QFontMetrics fm(font);
    const qreal maxTextWidth = rect.width() * 0.45;
    const QString elided = fm.elidedText(attribution, Qt::ElideRight, qMax(qreal(80.0) * scale, maxTextWidth));

    const qreal pad = 6.0 * scale;
    QRectF captionRect(QPointF(0, 0), QSizeF(fm.horizontalAdvance(elided) + pad * 2.0, fm.height() + pad * 2.0));
    const qreal margin = 4.0 * scale;
    if (anchor.testFlag(Qt::AlignLeft)) {
        captionRect.moveLeft(rect.left() + margin);
    } else {
        captionRect.moveRight(rect.right() - margin);
    }
    if (anchor.testFlag(Qt::AlignTop)) {
        captionRect.moveTop(rect.top() + margin);
    } else {
        captionRect.moveBottom(rect.bottom() - margin);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 120));
    painter.drawRoundedRect(captionRect, 4, 4);

    painter.setPen(QColor(255, 255, 255, 220));
    painter.drawText(captionRect, Qt::AlignCenter, elided);

    painter.restore();

    return captionRect;
}
