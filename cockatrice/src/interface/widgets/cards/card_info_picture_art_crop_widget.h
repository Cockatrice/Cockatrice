/**
 * @file card_info_picture_art_crop_widget.h
 * @ingroup CardWidgets
 * @brief TODO: Document this.
 */

#ifndef CARD_INFO_PICTURE_ART_CROP_WIDGET_H
#define CARD_INFO_PICTURE_ART_CROP_WIDGET_H

#include "card_info_picture_widget.h"

class CardInfoPictureArtCropWidget : public CardInfoPictureWidget
{
    Q_OBJECT

public:
    explicit CardInfoPictureArtCropWidget(QWidget *parent = nullptr);

    // Returns a processed (cropped & scaled) version of the pixmap
    QPixmap getProcessedBackground(const QSize &targetSize);
};

#endif // CARD_INFO_PICTURE_ART_CROP_WIDGET_H
