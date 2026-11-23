#include "card_art_utils.h"

#include "libcockatrice/card/card_info.h"

#include <QRect>
#include <QSize>
#include <QTransform>
#include <libcockatrice/card/printing/exact_card.h>
#include <qnamespace.h>

namespace CardArtUtils
{
QPixmap rotateSidewaysLayoutArt(const QPixmap &art, const ExactCard &card)
{
    if (!card.getInfo().getUiAttributes().landscapeOrientation) {
        return art;
    }

    QTransform transform;
    transform.rotate(90);
    return art.transformed(transform, Qt::SmoothTransformation);
}

QPixmap cropCardArt(const QPixmap &art, const ExactCard &card)
{
    if (art.isNull()) {
        return {};
    }

    const QPixmap source = rotateSidewaysLayoutArt(art, card);
    const bool landscape = card.getInfo().getUiAttributes().landscapeOrientation;
    const QSize size = source.size();
    const int marginX = size.width() * 0.07;
    // Landscape name boxes extend below 5% of the upright image. Remove the
    // whole box here, rather than relying on the banner's narrow viewport to
    // hide it: the same saved offset is also used by the taller profile popup.
    const int topMargin = size.height() * (landscape ? 0.15 : 0.11);
    const int bottomMargin = size.height() * 0.45;
    const QRect crop(marginX, topMargin, size.width() - 2 * marginX, size.height() - topMargin - bottomMargin);
    return source.copy(crop.intersected(source.rect()));
}
} // namespace CardArtUtils
