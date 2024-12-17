#include "card_amount_widget.h"
#include "../general/display/dynamic_font_size_push_button.h"
#include <QTimer>

/**
 * @brief Constructs a widget for displaying and controlling the card count in a specific zone.
 *
 * @param parent The parent widget.
 * @param deckEditor Pointer to the TabDeckEditor instance.
 * @param deckModel Pointer to the DeckListModel instance.
 * @param deckView Pointer to the QTreeView displaying the deck.
 * @param cardSizeSlider Pointer to the QSlider for adjusting font size.
 * @param rootCard The root card to manage within the widget.
 * @param setInfoForCard Card set information for the root card.
 * @param zoneName The zone name (e.g., DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
CardAmountWidget::CardAmountWidget(QWidget *parent,
                                   TabDeckEditor *deckEditor,
                                   DeckListModel *deckModel,
                                   QTreeView *deckView,
                                   QSlider *cardSizeSlider,
                                   CardInfoPtr &rootCard,
                                   CardInfoPerSet &setInfoForCard,
                                   QString zoneName)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), cardSizeSlider(cardSizeSlider),
      rootCard(rootCard), setInfoForCard(setInfoForCard), zoneName(zoneName), hovered(false)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->setAlignment(Qt::AlignHCenter);

    incrementButton = new DynamicFontSizePushButton(this);
    incrementButton->setTextAndColor("+", Qt::white);
    decrementButton = new DynamicFontSizePushButton(this);
    decrementButton->setTextAndColor("-", Qt::white);

    incrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
    decrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);

    // Set up connections based on the zone (Mainboard or Sideboard)
    if (zoneName == DECK_ZONE_MAIN) {
        connect(incrementButton, &QPushButton::clicked, this, &CardAmountWidget::addPrintingMainboard);
        connect(decrementButton, &QPushButton::clicked, this, &CardAmountWidget::removePrintingMainboard);
    } else if (zoneName == DECK_ZONE_SIDE) {
        connect(incrementButton, &QPushButton::clicked, this, &CardAmountWidget::addPrintingSideboard);
        connect(decrementButton, &QPushButton::clicked, this, &CardAmountWidget::removePrintingSideboard);
    }

    cardCountInZone = new QLabel(QString::number(countCardsInZone(zoneName)), this);
    cardCountInZone->setAlignment(Qt::AlignCenter);

    layout->addWidget(decrementButton);
    layout->addWidget(cardCountInZone);
    layout->addWidget(incrementButton);

    // React to model changes
    connect(deckModel, &DeckListModel::dataChanged, this, &CardAmountWidget::updateCardCount);
    connect(deckModel, &QAbstractItemModel::rowsRemoved, this, &CardAmountWidget::updateCardCount);

    // Connect slider for dynamic font size adjustment
    connect(cardSizeSlider, &QSlider::valueChanged, this, &CardAmountWidget::adjustFontSize);

    // Resize after a little delay to ensure parent size is available
    QTimer::singleShot(10, this, [this]() {
        adjustFontSize(this->cardSizeSlider->value());
        incrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
        decrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
        updateCardCount();
    });
}

/**
 * @brief Handles the painting of the widget, drawing a semi-transparent background.
 *
 * @param event The paint event.
 */
void CardAmountWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw semi-transparent black background
    painter.setBrush(QBrush(QColor(0, 0, 0, 128)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    QWidget::paintEvent(event);
}

/**
 * @brief Adjusts the font size of the card count label based on the slider value.
 *
 * @param scalePercentage The percentage value from the slider for scaling the font size.
 */
void CardAmountWidget::adjustFontSize(int scalePercentage)
{
    const int minFontSize = 8;      ///< Minimum font size
    const int maxFontSize = 32;     ///< Maximum font size
    const int basePercentage = 100; ///< Scale at 100%

    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / (250 - 25);
    newFontSize = std::clamp(newFontSize, minFontSize, maxFontSize);

    // Update the font for card count label
    QFont cardCountFont = cardCountInZone->font();
    cardCountFont.setPointSize(newFontSize);
    cardCountInZone->setFont(cardCountFont);

    incrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
    decrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);

    // Repaint the widget
    repaint();
}

/**
 * @brief Updates the card count display in the widget.
 */
void CardAmountWidget::updateCardCount()
{
    cardCountInZone->setText("<font color='white'>" + QString::number(countCardsInZone(zoneName)) + "</font>");
    layout->invalidate();
    layout->activate();
}

/**
 * @brief Adds a printing of the card to the specified zone (Mainboard or Sideboard).
 *
 * @param zone The zone to add the card to (DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
void CardAmountWidget::addPrinting(const QString &zone)
{
    auto newCardIndex = deckModel->addCard(rootCard->getName(), setInfoForCard, zone);
    recursiveExpand(newCardIndex);
    QModelIndex find_card = deckModel->findCard(rootCard->getName(), zone);
    if (find_card.isValid() && find_card != newCardIndex) {
        auto amount = deckModel->data(find_card, Qt::DisplayRole);
        if (amount.toInt() > 1) {
            for (int i = 0; i < amount.toInt() - 1; i++) {
                deckModel->addCard(rootCard->getName(), setInfoForCard, zone);
            }
        }
        deckModel->removeRow(find_card.row(), find_card.parent());
    };
    newCardIndex = deckModel->findCard(rootCard->getName(), zone, setInfoForCard.getProperty("uuid"),
                                       setInfoForCard.getProperty("num"));
    deckView->setCurrentIndex(newCardIndex);
    deckView->setFocus(Qt::FocusReason::MouseFocusReason);
}

/**
 * @brief Adds a printing to the mainboard zone.
 */
void CardAmountWidget::addPrintingMainboard()
{
    addPrinting(DECK_ZONE_MAIN);
}

/**
 * @brief Adds a printing to the sideboard zone.
 */
void CardAmountWidget::addPrintingSideboard()
{
    addPrinting(DECK_ZONE_SIDE);
}

/**
 * @brief Removes a printing from the mainboard zone.
 */
void CardAmountWidget::removePrintingMainboard()
{
    decrementCardHelper(DECK_ZONE_MAIN);
}

/**
 * @brief Removes a printing from the sideboard zone.
 */
void CardAmountWidget::removePrintingSideboard()
{
    decrementCardHelper(DECK_ZONE_SIDE);
}

/**
 * @brief Recursively expands the card in the deck view starting from the given index.
 *
 * @param index The model index of the card to expand.
 */
void CardAmountWidget::recursiveExpand(const QModelIndex &index)
{
    if (index.parent().isValid()) {
        recursiveExpand(index.parent());
    }
    deckView->expand(index);
}

/**
 * @brief Offsets the card count at the specified index by the given amount.
 *
 * @param idx The model index of the card.
 * @param offset The amount to add or subtract from the card count.
 */
void CardAmountWidget::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid() || offset == 0) {
        return;
    }

    const QModelIndex numberIndex = idx.sibling(idx.row(), 0);
    const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
    const int new_count = count + offset;
    deckView->setCurrentIndex(numberIndex);
    if (new_count <= 0) {
        deckModel->removeRow(idx.row(), idx.parent());
    } else {
        deckModel->setData(numberIndex, new_count, Qt::EditRole);
    }
    deckEditor->setModified(true);
}

/**
 * @brief Helper function to decrement the card count for a given zone.
 *
 * @param zone The zone from which to remove the card (DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
void CardAmountWidget::decrementCardHelper(const QString &zone)
{
    QModelIndex idx;
    idx = deckModel->findCard(rootCard->getName(), zone, setInfoForCard.getProperty("uuid"),
                              setInfoForCard.getProperty("num"));
    offsetCountAtIndex(idx, -1);
}

/**
 * @brief Counts the number of cards in a specific zone (mainboard or sideboard).
 *
 * @param deckZone The name of the zone (e.g., DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 * @return The number of cards in the zone.
 */
int CardAmountWidget::countCardsInZone(const QString &deckZone)
{
    int count = 0;

    if (setInfoForCard.getProperty("uuid").isEmpty()) {
        return 0; // Cards without uuids/providerIds CANNOT match another card, they are undefined for us.
    }

    if (!deckModel) {
        return -1;
    }

    DeckList *decklist = deckModel->getDeckList();
    if (!decklist) {
        return -1;
    }

    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot) {
        return -1;
    }

    for (auto *i : *listRoot) {
        auto *countCurrentZone = dynamic_cast<InnerDecklistNode *>(i);
        if (!countCurrentZone) {
            continue;
        }

        if (countCurrentZone->getName() != deckZone) {
            continue;
        }

        for (auto *cardNode : *countCurrentZone) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(cardNode);
            if (!currentCard) {
                continue;
            }

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                if (currentCard->getCardProviderId() == setInfoForCard.getProperty("uuid")) {
                    count++;
                }
            }
        }
    }
    return count;
}