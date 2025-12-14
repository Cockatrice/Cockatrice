#ifndef COCKATRICE_ARCHIDEKT_DECK_PREVIEW_IMAGE_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_DECK_PREVIEW_IMAGE_DISPLAY_WIDGET_H

#include "../../../../general/display/shadow_background_label.h"

#include <QLabel>
#include <QWidget>

/**
 * @class ArchidektDeckPreviewImageDisplayWidget
 * @brief Widget for displaying a deck preview image with overlaid metadata labels.
 *
 * This widget shows a deck's preview image along with several overlay labels:
 *  - Top-left: Deck name.
 *  - Top-right: Card count.
 *  - Bottom-left: EDH bracket (if applicable).
 *  - Bottom-right: View count.
 *
 * Labels automatically scale and position themselves relative to the widget's size.
 * The image can be scaled while maintaining a specified aspect ratio.
 *
 * ### Features
 * - Adjustable preview width and aspect ratio.
 * - Labels automatically repositioned on resize.
 * - Supports overlaying multiple pieces of metadata with shadowed labels for readability.
 */
class ArchidektDeckPreviewImageDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the deck preview display widget.
     * @param parent Optional parent widget.
     */
    explicit ArchidektDeckPreviewImageDisplayWidget(QWidget *parent = nullptr);

    /**
     * @brief Sets the aspect ratio for the preview image (height / width).
     * @param ratio Aspect ratio to maintain.
     */
    void setAspectRatio(float ratio);

    /**
     * @brief Sets the width of the preview image; height is adjusted according to the aspect ratio.
     * @param width Desired width in pixels.
     */
    void setPreviewWidth(int width);

    QLabel *imageLabel;                      ///< QLabel to display the deck image
    ShadowBackgroundLabel *topLeftLabel;     ///< Overlay label at top-left (deck name)
    ShadowBackgroundLabel *topRightLabel;    ///< Overlay label at top-right (card count)
    ShadowBackgroundLabel *bottomLeftLabel;  ///< Overlay label at bottom-left (EDH bracket)
    ShadowBackgroundLabel *bottomRightLabel; ///< Overlay label at bottom-right (views)

protected:
    /**
     * @brief Handles resize events to reposition the image and overlay labels.
     * @param event Resize event.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    float aspectRatio = 1.0f; ///< Aspect ratio to maintain for the preview image
};

#endif // COCKATRICE_ARCHIDEKT_DECK_PREVIEW_IMAGE_DISPLAY_WIDGET_H
