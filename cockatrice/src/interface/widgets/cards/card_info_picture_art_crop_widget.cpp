#include "card_info_picture_art_crop_widget.h"

#include "../../../interface/card_picture_loader/card_picture_loader.h"

CardInfoPictureArtCropWidget::CardInfoPictureArtCropWidget(QWidget *parent)
    : CardInfoPictureWidget(parent, false, false)
{
    hide();
}

QPixmap CardInfoPictureArtCropWidget::getBackground()
{
    // Load the full-resolution card image, not a pre-scaled one
    QPixmap fullResPixmap;
    if (getCard()) {
        CardPictureLoader::getPixmap(fullResPixmap, getCard(), QSize(745, 1040)); // or a high default size
    } else {
        CardPictureLoader::getCardBackPixmap(fullResPixmap, QSize(745, 1040));
    }
    if (fullResPixmap.isNull()) {
        return QPixmap(); // return null qpixmap
    }

    const QSize sz = fullResPixmap.size();

    int marginX = sz.width() * 0.07;
    int topMargin = sz.height() * 0.11;
    int bottomMargin = sz.height() * 0.45;

    QRect foilRect(marginX, topMargin, sz.width() - 2 * marginX, sz.height() - topMargin - bottomMargin);

    foilRect = foilRect.intersected(fullResPixmap.rect()); // always clamp to source bounds

    // return full resolution image crop
    QPixmap cropped = fullResPixmap.copy(foilRect);
    return cropped;
}
