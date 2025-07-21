#include "all_zones_card_amount_widget.h"

#include "../general/display/shadow_background_label.h"

#include <QTimer>

/**
 * @brief Constructor for the AllZonesCardAmountWidget class.
 *
 * Initializes the widget with its layout and sets up the connections and necessary
 * UI elements for managing card counts in both the mainboard and sideboard zones.
 *
 * @param parent The parent widget.
 * @param deckEditor Pointer to the TabDeckEditor.
 * @param deckModel Pointer to the DeckListModel.
 * @param deckView Pointer to the QTreeView for the deck display.
 * @param cardSizeSlider Pointer to the QSlider used for dynamic font resizing.
 * @param rootCard The root card for the widget.
 */
AllZonesCardAmountWidget::AllZonesCardAmountWidget(QWidget *parent,
                                                   AbstractTabDeckEditor *deckEditor,
                                                   DeckListModel *deckModel,
                                                   QTreeView *deckView,
                                                   QSlider *cardSizeSlider,
                                                   const ExactCard &rootCard)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), cardSizeSlider(cardSizeSlider),
      rootCard(rootCard)
{
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContentsMargins(5, 5, 5, 5); // Padding around the text

    zoneLabelMainboard = new ShadowBackgroundLabel(this, tr("Mainboard"));
    buttonBoxMainboard =
        new CardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard, DECK_ZONE_MAIN);
    zoneLabelSideboard = new ShadowBackgroundLabel(this, tr("Sideboard"));
    buttonBoxSideboard =
        new CardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard, DECK_ZONE_SIDE);

    layout->addWidget(zoneLabelMainboard, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(buttonBoxMainboard, 0, Qt::AlignHCenter | Qt::AlignTop);
    layout->addSpacing(25);
    layout->addWidget(zoneLabelSideboard, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(buttonBoxSideboard, 0, Qt::AlignHCenter | Qt::AlignTop);

    connect(cardSizeSlider, &QSlider::valueChanged, this, &AllZonesCardAmountWidget::adjustFontSize);

    QTimer::singleShot(10, this, [this]() { adjustFontSize(this->cardSizeSlider->value()); });

    setMouseTracking(true);
}

/**
 * @brief Adjusts the font size of the zone labels based on the slider value.
 *
 * This method calculates the new font size as a percentage of the original font size
 * based on the slider value and applies it to the zone label text.
 *
 * @param scalePercentage The scale percentage from the slider.
 */
void AllZonesCardAmountWidget::adjustFontSize(int scalePercentage)
{
    const int minFontSize = 8;      // Minimum font size
    const int maxFontSize = 32;     // Maximum font size
    const int basePercentage = 100; // Scale at 100%

    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / 225;
    newFontSize = std::clamp(newFontSize, minFontSize, maxFontSize);

    // Update the font labels
    QFont zoneLabelFont = zoneLabelMainboard->font();
    zoneLabelFont.setPointSize(newFontSize);
    zoneLabelMainboard->setFont(zoneLabelFont);
    zoneLabelSideboard->setFont(zoneLabelFont);

    // Repaint the widget (if necessary)
    repaint();
}

/**
 * @brief Gets the card count in the mainboard zone.
 *
 * @return The number of cards in the mainboard.
 */
int AllZonesCardAmountWidget::getMainboardAmount()
{
    return buttonBoxMainboard->countCardsInZone(DECK_ZONE_MAIN);
}

/**
 * @brief Gets the card count in the sideboard zone.
 *
 * @return The number of cards in the sideboard.
 */
int AllZonesCardAmountWidget::getSideboardAmount()
{
    return buttonBoxSideboard->countCardsInZone(DECK_ZONE_SIDE);
}

/**
 * @brief Handles the event when the mouse enters the widget.
 *
 * This method is triggered when the mouse enters the widget's area, allowing for updates
 * or interactions such as UI feedback or layout changes.
 *
 * @param event The event information for the mouse entry.
 */
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void AllZonesCardAmountWidget::enterEvent(QEnterEvent *event)
#else
void AllZonesCardAmountWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    update();
}
