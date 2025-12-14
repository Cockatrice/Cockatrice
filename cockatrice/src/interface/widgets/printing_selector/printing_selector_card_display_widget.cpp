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
 * @param _deckEditor The TabDeckEditor instance for deck management.
 * @param _deckModel The DeckListModel instance providing deck data.
 * @param _deckView The QTreeView instance displaying the deck.
 * @param _cardSizeSlider The slider controlling the size of the displayed card.
 * @param _rootCard The root card object, representing the card to be displayed.
 * @param _currentZone The current zone in which the card is located.
 */
PrintingSelectorCardDisplayWidget::PrintingSelectorCardDisplayWidget(QWidget *parent,
                                                                     AbstractTabDeckEditor *_deckEditor,
                                                                     DeckListModel *_deckModel,
                                                                     QTreeView *_deckView,
                                                                     QSlider *_cardSizeSlider,
                                                                     const ExactCard &_rootCard,
                                                                     QString &_currentZone)
    : QWidget(parent), deckEditor(_deckEditor), deckModel(_deckModel), deckView(_deckView),
      cardSizeSlider(_cardSizeSlider), rootCard(_rootCard), currentZone(_currentZone)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create the overlay widget for the card display
    overlayWidget =
        new PrintingSelectorCardOverlayWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard);
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
