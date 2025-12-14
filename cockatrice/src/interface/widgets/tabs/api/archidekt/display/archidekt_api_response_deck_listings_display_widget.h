#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_LISTINGS_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_LISTINGS_DISPLAY_WIDGET_H

#include "../../../../cards/card_size_widget.h"
#include "../../../../general/layout_containers/flow_widget.h"
#include "../api_response/archidekt_deck_listing_api_response.h"

#include <QNetworkAccessManager>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @class ArchidektApiResponseDeckListingsDisplayWidget
 * @brief Displays a scrollable horizontal list of Archidekt deck listings with dynamic card sizing.
 *
 * This widget serves as a container for multiple
 * ArchidektApiResponseDeckEntryDisplayWidget instances, each representing one deck listing
 * returned from an Archidekt API call.
 *
 * ### Responsibilities
 * - Creates a **FlowWidget** that arranges deck entries horizontally with wrapping.
 * - Creates a shared **QNetworkAccessManager** for entry widgets to fetch card thumbnails.
 * - Connects a **CardSizeWidget** slider to all deck entries to dynamically rescale their preview cards.
 * - Propagates deck-navigation requests (`requestNavigation`) from children to the parent.
 *
 * ### Layout
 * The widget uses a single `QHBoxLayout` containing the `FlowWidget`.
 * The FlowWidget automatically manages child flow and scrollbar behavior (horizontal = off,
 * vertical = auto), providing an efficient scrollable gallery of deck entries.
 *
 * ### API Integration
 * The constructor consumes an `ArchidektDeckListingApiResponse`, iterates through its
 * `results`, and instantiates a child entry widget for each deck.
 *
 * ### Signals
 * - `requestNavigation(QString url)` — emitted whenever a child entry widget requests
 *   navigation to a deck or related Archidekt page.
 *
 * ### Performance Notes
 * - All entry widgets share a single QNetworkAccessManager instance to reuse connections
 *   and avoid redundant session creation.
 * - `resizeEvent()` forces layout invalidation to ensure the flow layout responds properly
 *   to container resizing.
 */
class ArchidektApiResponseDeckListingsDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    /**
     * @brief Emitted when a child deck entry requests that the UI navigate to a particular URL.
     * @param url The destination URL (typically an Archidekt deck page).
     */
    void requestNavigation(QString url);

public:
    /**
     * @brief Constructs a widget that displays multiple deck listing previews.
     *
     * @param parent Parent widget.
     * @param response The Archidekt API response containing deck listings.
     * @param cardSizeSlider A slider widget used to dynamically resize card previews.
     *
     * Each deck in `response.results` becomes its own
     * ArchidektApiResponseDeckEntryDisplayWidget, added to the FlowWidget.
     */
    explicit ArchidektApiResponseDeckListingsDisplayWidget(QWidget *parent,
                                                           ArchidektDeckListingApiResponse response,
                                                           CardSizeWidget *cardSizeSlider);

    /**
     * @brief Ensures FlowWidget layout properly recomputes on resize.
     *
     * Forces the layout to invalidate and activate itself so that the
     * FlowWidget recalculates wrapping and child placement.
     *
     * @param event Resize event.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    /// Slider controlling the scale of card thumbnails in all deck entry widgets.
    CardSizeWidget *cardSizeSlider;

    /// Main horizontal layout containing the FlowWidget.
    QHBoxLayout *layout;

    /// Container providing scrollable multi-row flow layout of deck entries.
    FlowWidget *flowWidget;

    /// Shared network manager used to download card images for all child entry widgets.
    QNetworkAccessManager *imageNetworkManager;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_LISTINGS_DISPLAY_WIDGET_H