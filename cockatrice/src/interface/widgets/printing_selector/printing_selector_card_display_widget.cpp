#include "printing_selector_card_display_widget.h"

#include "printing_selector_card_overlay_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <QGraphicsEffect>
#include <QVBoxLayout>
#include <utility>

/**
 * @brief Constructs a PrintingSelectorCardDisplayWidget to display card information.
 *
 * This widget is responsible for displaying the selected card's printing information, including
 * the card's image and set details. It also handles the layout of the card's display, including
 * its size, set name, and collectors number. The card is displayed within a `QVBoxLayout` with
 * two main components: the overlay (which combines the card image and buttons) and the set name and collectors number
 * display.
 *
 * @param parent The parent widget for this display.
 * @param deckEditor The TabDeckEditor instance for deck management.
 * @param deckStateManager The DeckStateManager instance providing deck data.
 * @param cardSizeSlider The slider controlling the size of the displayed card.
 * @param rootCard The root card object, representing the card to be displayed.
 */
PrintingSelectorCardDisplayWidget::PrintingSelectorCardDisplayWidget(QWidget *parent,
                                                                     AbstractTabDeckEditor *deckEditor,
                                                                     DeckStateManager *deckStateManager,
                                                                     QSlider *cardSizeSlider,
                                                                     const ExactCard &rootCard)
    : QWidget(parent), rootCard(rootCard)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create the overlay widget for the card display
    overlayWidget = new PrintingSelectorCardOverlayWidget(this, deckEditor, deckStateManager, cardSizeSlider, rootCard);
    connect(overlayWidget, &PrintingSelectorCardOverlayWidget::cardPreferenceChanged, this,
            [this]() { emit cardPreferenceChanged(); });

    CardSetPtr set = rootCard.getPrinting().getSet();

    // Create the widget to display the set name and collector's number
    QString combinedSetName = QString(set->getLongName() + " (" + set->getShortName() + ")");
    setNameAndCollectorsNumberDisplayWidget = new SetNameAndCollectorsNumberDisplayWidget(
        this, combinedSetName, rootCard.getPrinting().getProperty("num"), cardSizeSlider);

    // Add the widgets to the layout
    layout->addWidget(overlayWidget, 0, Qt::AlignHCenter);
    layout->addWidget(setNameAndCollectorsNumberDisplayWidget, 1, Qt::AlignHCenter | Qt::AlignBottom);
}

/**
 * @brief Adjusts the width of the set name display to fit the card overlay widget.
 *
 * This method ensures that the set name and collector's number display widget does not exceed
 * the width of the card's overlay widget. It clamps the set name widget to match the width of
 * the overlay widget and updates the display.
 */
void PrintingSelectorCardDisplayWidget::clampSetNameToPicture()
{
    if (overlayWidget != nullptr && setNameAndCollectorsNumberDisplayWidget != nullptr) {
        setNameAndCollectorsNumberDisplayWidget->setMaximumWidth(overlayWidget->width());
    }
    update();
}

void PrintingSelectorCardDisplayWidget::updateCardAmounts(const QMap<QString, QPair<int, int>> &uuidToAmounts)
{
    auto [main, side] = uuidToAmounts.value(rootCard.getPrinting().getUuid());
    overlayWidget->updateCardAmounts(main, side);
}
