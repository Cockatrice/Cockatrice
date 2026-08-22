#ifndef COCKATRICE_ART_CROP_ATTRIBUTION_H
#define COCKATRICE_ART_CROP_ATTRIBUTION_H

#include <QStringView>

class ExactCard;
class QPainter;
class QRectF;
class QString;

/**
 * @brief Builds an attribution caption for a cropped card art display.
 *
 * When a card image's art region is shown cropped (an "art crop"), the artist
 * should be credited in the same interface. Returns an empty string when the
 * database has no artist data for the card.
 *
 * @param card The card whose art is being displayed.
 * @return Caption such as "Art: John Avon", or empty.
 */
QString buildArtAttribution(const ExactCard &card);

/**
 * @brief Paints an attribution caption in a corner of a rect.
 *
 * Draws a subtle semi-transparent pill containing the caption, elided to fit.
 *
 * @param painter Painter to draw with.
 * @param rect The area (e.g. the cropped art region) the caption belongs to.
 * @param attribution Caption text (see buildArtAttribution()).
 * @param anchor Corner of @p rect to pin the pill to (default bottom-right).
 * @param scale Size multiplier for the pill (e.g. 0.8 for a smaller pill).
 * @return The rect the pill was drawn in, or an empty rect if @p attribution is empty.
 */
QRectF paintArtAttribution(QPainter &painter,
                           const QRectF &rect,
                           const QString &attribution,
                           Qt::Alignment anchor = Qt::AlignRight | Qt::AlignBottom,
                           qreal scale = 1.0);

#endif // COCKATRICE_ART_CROP_ATTRIBUTION_H
