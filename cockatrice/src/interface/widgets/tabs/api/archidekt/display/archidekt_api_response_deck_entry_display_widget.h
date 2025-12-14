#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../api_response/deck_listings/archidekt_api_response_deck_listing_container.h"
#include "archidekt_deck_preview_image_display_widget.h"

#include <QLabel>
#include <QNetworkAccessManager>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

class BackgroundPlateWidget;

/**
 * @class ArchidektApiResponseDeckEntryDisplayWidget
 * @brief Displays a single Archidekt deck listing as a preview card with metadata.
 *
 * This widget renders a deck entry received from an Archidekt API response. It includes:
 *  - A scaled deck preview image loaded asynchronously via QNetworkAccessManager.
 *  - Elided deck name in the top-left corner.
 *  - Deck size, EDH bracket, and view count labels.
 *  - A color distribution bar summarizing deck colors.
 *  - Metadata labels including owner, creation date, and last update date.
 *
 * The widget dynamically scales the preview image and labels according to a linked
 * CardSizeWidget slider. Hovering over the widget highlights the background plate,
 * and clicking emits a `requestNavigation` signal pointing to the deck URL.
 *
 * ### Features
 * - Asynchronous image loading with fallback to a default placeholder image.
 * - Maintains a fixed aspect ratio for the preview image (150:267).
 * - Updates text elision dynamically when resized or scaled.
 * - Integrates with FlowWidget containers for scrollable deck galleries.
 *
 * ### Signals
 * - `requestNavigation(QString url)` — emitted when the widget is clicked to request
 *   navigation to the deck's Archidekt page.
 *
 * ### Slots
 * - `actRequestNavigationToDeck()` — triggers navigation.
 * - `setScaleFactor(int scale)` — adjusts preview image scaling.
 */
class ArchidektApiResponseDeckEntryDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    /**
     * @brief Emitted when the user requests navigation.
     * @param url Full URL to the Archidekt page.
     */
    void requestNavigation(QString url);

public:
    /**
     * @brief Constructs a deck entry display widget.
     * @param parent Parent widget.
     * @param response API container holding deck listing data.
     * @param imageNetworkManager Shared network manager for fetching preview images.
     */
    explicit ArchidektApiResponseDeckEntryDisplayWidget(QWidget *parent,
                                                        ArchidektApiResponseDeckListingContainer response,
                                                        QNetworkAccessManager *imageNetworkManager);

    /**
     * @brief Handles finished network replies for preview images.
     * @param reply QNetworkReply containing image data.
     *
     * Validates that the reply corresponds to this widget and updates the preview image.
     */
    void onPreviewImageLoadFinished(QNetworkReply *reply);

    /**
     * @brief Updates the scaled preview image and adjusts layout accordingly.
     */
    void updateScaledPreview();

    /**
     * @brief Ensures layout responds correctly on resize events.
     * @param event Resize event.
     */
    void resizeEvent(QResizeEvent *event) override;

public slots:
    /**
     * @brief Emits `requestNavigation` for the deck's URL.
     */
    void actRequestNavigationToDeck();

    /**
     * @brief Sets a scaling factor (percentage) for the preview image.
     * @param scale Scale percentage (100 = normal size).
     */
    void setScaleFactor(int scale);

protected:
    void mousePressEvent(QMouseEvent *event) override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override; ///< Qt6 hover enter
#else
    void enterEvent(QEvent *event) override; ///< Qt5 hover enter
#endif
    void leaveEvent(QEvent *event) override;

private:
    QVBoxLayout *layout;                                   ///< Main vertical layout
    ArchidektApiResponseDeckListingContainer response;     ///< Deck data
    QUrl imageUrl;                                         ///< URL of the deck's preview image
    QNetworkAccessManager *imageNetworkManager;            ///< Shared network manager
    ArchidektDeckPreviewImageDisplayWidget *previewWidget; ///< Widget showing the deck preview
    QLabel *picture;                                       ///< QLabel displaying the scaled pixmap
    QPixmap originalPixmap;                                ///< Original image for scaling (avoids degradation)
    int scaleFactor = 100;                                 ///< Current scaling percentage
    BackgroundPlateWidget *backgroundPlateWidget;          ///< Plate for metadata labels
    static constexpr float DESIGN_RATIO = 150.0f / 267.0f; ///< Design aspect ratio
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
