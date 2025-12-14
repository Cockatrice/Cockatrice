#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_DISPLAY_WIDGET_H

#include "../../../../../deck_loader/deck_loader.h"
#include "../../../../cards/card_size_widget.h"
#include "../../../../general/layout_containers/flow_widget.h"
#include "../../../../visual_deck_editor/visual_deck_display_options_widget.h"
#include "../api_response/deck/archidekt_api_response_deck.h"
#include "deck_list_model.h"

#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @class ArchidektApiResponseDeckDisplayWidget
 * @brief Displays a full deck fetched from an Archidekt API response.
 *
 * This widget visualizes all cards in a deck retrieved from the Archidekt API.
 * It supports:
 *  - Interactive display options via a VisualDeckDisplayOptionsWidget.
 *  - Scrollable display of deck zones/cards with DeckCardZoneDisplayWidget.
 *  - Integration with a CardSizeWidget slider for card scaling.
 *  - Opening the deck in a deck editor.
 *
 * The widget internally constructs a DeckListModel from the Archidekt API response,
 * then builds zone widgets for each group of cards according to the active group
 * criteria. It also responds dynamically to model resets or sorting/grouping changes.
 *
 * ### Signals
 * - `requestNavigation(QString url)` — triggered when navigation to a deck URL is requested.
 * - `openInDeckEditor(DeckLoader *loader)` — emitted when the user chooses to open the deck
 *   in the deck editor.
 *
 * ### Features
 * - Automatically generates DeckCardZoneDisplayWidget instances for each card group.
 * - Provides a scrollable layout for decks of arbitrary size.
 * - Updates layouts dynamically when resized or when display/group/sort criteria change.
 */
class ArchidektApiResponseDeckDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    /**
     * @brief Emitted when navigation to a deck URL is requested.
     * @param url URL of the deck on Archidekt.
     */
    void requestNavigation(QString url);

    /**
     * @brief Emitted when the deck should be opened in the deck editor.
     * @param loader Initialized DeckLoader containing the deck data.
     */
    void openInDeckEditor(DeckLoader *loader);

public:
    /**
     * @brief Constructs a display widget for an Archidekt deck.
     * @param parent Parent widget.
     * @param response API deck data container.
     * @param cardSizeSlider Slider controlling card scaling.
     */
    explicit ArchidektApiResponseDeckDisplayWidget(QWidget *parent,
                                                   ArchidektApiResponseDeck response,
                                                   CardSizeWidget *cardSizeSlider);

    /**
     * @brief Updates all UI text for retranslation/localization.
     *
     * Called when the application language changes.
     */
    void retranslateUi();

    /**
     * @brief Opens the deck in the deck editor via DeckLoader.
     */
    void actOpenInDeckEditor();

    /**
     * @brief Clears all dynamically generated card zone display widgets.
     */
    void clearAllDisplayWidgets();

    /**
     * @brief Handles model reset by clearing and reconstructing display widgets.
     */
    void decklistModelReset();

    /**
     * @brief Builds DeckCardZoneDisplayWidget instances from the current DeckListModel.
     */
    void constructZoneWidgetsFromDeckListModel();

private slots:
    /**
     * @brief Slot triggered when the active group criteria change.
     * @param activeGroupCriteria Name of the new grouping criteria.
     */
    void onGroupCriteriaChange(const QString &activeGroupCriteria);

private:
    ArchidektApiResponseDeck response;                        ///< API deck data container
    CardSizeWidget *cardSizeSlider;                           ///< Slider for adjusting card sizes
    QVBoxLayout *layout;                                      ///< Main vertical layout
    QPushButton *openInEditorButton;                          ///< Button to open deck in editor
    VisualDeckDisplayOptionsWidget *displayOptionsWidget;     ///< Controls grouping/sorting/display
    QScrollArea *scrollArea;                                  ///< Scrollable area for deck zones
    QWidget *zoneContainer;                                   ///< Container for deck zones
    QVBoxLayout *zoneContainerLayout;                         ///< Layout for deck zones
    QWidget *container;                                       ///< Outer container for scroll area
    QHash<QPersistentModelIndex, QWidget *> indexToWidgetMap; ///< Maps model indices to widgets
    QVBoxLayout *containerLayout;                             ///< Layout for container
    DeckListModel *model;                                     ///< Deck list model
protected slots:
    /**
     * @brief Updates layout and display on resize.
     * @param event Resize event.
     */
    void resizeEvent(QResizeEvent *event) override;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_DISPLAY_WIDGET_H
