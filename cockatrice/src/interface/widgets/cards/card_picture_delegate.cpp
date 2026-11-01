#include "card_picture_delegate.h"

#include "../../../client/settings/cache_settings.h"
#include "../../card_picture_loader/card_picture_loader.h"
#include "card_art_utils.h"
#include "card_database_visual_display_model.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/settings/cards_display_settings.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRect>
#include <QStyleOptionViewItem>
#include <libcockatrice/card/printing/printing_info.h>

CardPictureDelegate::CardPictureDelegate(CardDatabaseVisualDisplayModel *_model, QObject *parent)
    : QStyledItemDelegate(parent), model(_model)
{
}

void CardPictureDelegate::setCardSize(const QSize &size)
{
    if (cardSize == size) {
        return;
    }

    cardSize = size;
}

void CardPictureDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (model == nullptr) {
        return;
    }

    const ExactCard card = model->cardAt(index.row());
    if (!card) {
        return;
    }

    // The grid cell is the card plus SPACING; give the picture the rest.
    const int inset = SPACING / 2;
    const QRect cardArea = option.rect.adjusted(inset, inset, -inset, -inset);

    QPixmap pixmap;
    CardPictureLoader::getCardBackLoadingInProgressPixmap(pixmap, cardArea.size());
    CardPictureLoader::getPixmap(pixmap, card, cardArea.size());

    if (pixmap.isNull()) {
        return;
    }

    if (SettingsCache::instance().cardsDisplay().getAutoRotateSidewaysLayoutCards()) {
        pixmap = CardArtUtils::rotateSidewaysLayoutArt(pixmap, card);
    }

    // Fit the picture into the cell and center it, like CardInfoPictureWidget does.
    const QSize pictureSize = pixmap.deviceIndependentSize().toSize().scaled(cardArea.size(), Qt::KeepAspectRatio);
    const QRect pictureRect(cardArea.center().x() - pictureSize.width() / 2,
                            cardArea.center().y() - pictureSize.height() / 2, pictureSize.width(),
                            pictureSize.height());

    const qreal radius = SettingsCache::instance().cardsDisplay().getRoundCardCorners()
                             ? 0.05 * static_cast<qreal>(pictureRect.width())
                             : 0.0;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath shape;
    shape.addRoundedRect(pictureRect, radius, radius);
    painter->setClipPath(shape);
    // Source rect is in device pixels, target in logical ones: drawPixmap scales
    // between them without the device pixel ratio being applied twice.
    painter->drawPixmap(pictureRect, pixmap, QRectF(pixmap.rect()));
    painter->restore();
}

QSize CardPictureDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
    return cardSize + QSize(SPACING, SPACING);
}
