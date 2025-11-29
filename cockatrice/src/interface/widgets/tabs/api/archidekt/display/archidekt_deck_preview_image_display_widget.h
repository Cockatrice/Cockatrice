//
// Created by Ascor on 25-Nov-25.
//

#ifndef COCKATRICE_ARCHIDEKT_DECK_PREVIEW_IMAGE_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_DECK_PREVIEW_IMAGE_DISPLAY_WIDGET_H

#include "../../../../general/display/shadow_background_label.h"

#include <QLabel>
#include <QWidget>

class ArchidektDeckPreviewImageDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ArchidektDeckPreviewImageDisplayWidget(QWidget *parent = nullptr);

    void setAspectRatio(float ratio);
    void setPreviewWidth(int width);

    QLabel *imageLabel;
    ShadowBackgroundLabel *topLeftLabel;
    ShadowBackgroundLabel *topRightLabel;
    ShadowBackgroundLabel *bottomLeftLabel;
    ShadowBackgroundLabel *bottomRightLabel;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    float aspectRatio = 1.0f;
};

#endif // COCKATRICE_ARCHIDEKT_DECK_PREVIEW_IMAGE_DISPLAY_WIDGET_H
