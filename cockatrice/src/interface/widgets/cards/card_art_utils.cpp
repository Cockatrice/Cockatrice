#include "card_art_utils.h"

#include "libcockatrice/card/card_info.h"

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
} // namespace CardArtUtils