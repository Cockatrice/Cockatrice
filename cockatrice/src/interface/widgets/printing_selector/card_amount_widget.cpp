#include "card_amount_widget.h"

#include <QPainter>
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
 * @param zoneName The zone name (e.g., DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
CardAmountWidget::CardAmountWidget(QWidget *parent,
                                   AbstractTabDeckEditor *deckEditor,
                                   DeckListModel *deckModel,
                                   QTreeView *deckView,
                                   QSlider *cardSizeSlider,
                                   const ExactCard &rootCard,
                                   const QString &zoneName)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), cardSizeSlider(cardSizeSlider),
      rootCard(rootCard), zoneName(zoneName), hovered(false)
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

    if (deckEditor) {
        connect(this, &CardAmountWidget::deckModified, deckEditor, &AbstractTabDeckEditor::onDeckHistorySaveRequested);
    }
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

void CardAmountWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    adjustFontSize(this->cardSizeSlider->value());
    updateCardCount();

    if (parentWidget()) {
        int width = parentWidget()->size().width();
        int height = parentWidget()->size().height();

        incrementButton->setFixedSize(width / 3, height / 9);
        decrementButton->setFixedSize(width / 3, height / 9);
    }
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

    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / 225;
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
    int addedCount = 1;
    // Check if we will need to add extra copies due to replacing copies without providerIds
    QModelIndex existing = deckModel->findCard(rootCard.getName(), zone);
    int extraCopies = 0;
    bool replacingProviderless = false;

    if (existing.isValid()) {
        QString providerId =
            existing.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::DisplayRole).toString();
        if (providerId.isEmpty()) {
            int amount = existing.data(Qt::DisplayRole).toInt();
            extraCopies = amount - 1; // One less because we *always* add one
            replacingProviderless = true;
        }
    }

    addedCount += extraCopies;

    QString reason = QString("Added %1 copies of '%2 (%3) %4' to %5 [ProviderID: %6]%7")
                         .arg(addedCount)
                         .arg(rootCard.getName())
                         .arg(rootCard.getPrinting().getSet()->getShortName())
                         .arg(rootCard.getPrinting().getProperty("num"))
                         .arg(zone == DECK_ZONE_MAIN ? "mainboard" : "sideboard")
                         .arg(rootCard.getPrinting().getUuid())
                         .arg(replacingProviderless ? " (replaced providerless printings)" : "");

    emit deckModified(reason);

    // Add the card and expand the list UI
    auto newCardIndex = deckModel->addCard(rootCard, zone);

    // Check if a card without a providerId already exists in the deckModel and replace it, if so.
    QString foundProviderId =
        existing.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::DisplayRole).toString();
    if (existing.isValid() && existing != newCardIndex && foundProviderId == "") {
        auto amount = existing.data(Qt::DisplayRole);
        for (int i = 0; i < amount.toInt() - 1; i++) {
            deckModel->addCard(rootCard, zone);
        }
        deckModel->removeRow(existing.row(), existing.parent());
    }

    // Set Index and Focus as if the user had just clicked the new card and modify the deckEditor saveState
    newCardIndex = deckModel->findCard(rootCard.getName(), zone, rootCard.getPrinting().getUuid(),
                                       rootCard.getPrinting().getProperty("num"));

    deckView->setCurrentIndex(newCardIndex);
    deckView->setFocus(Qt::FocusReason::MouseFocusReason);
    deckEditor->setModified(true);
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
 * @brief Helper function to decrement the card count for a given zone.
 *
 * @param zone The zone from which to remove the card (DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
void CardAmountWidget::decrementCardHelper(const QString &zone)
{
    QString reason = QString("Removed 1 copy of '%1 (%2) %3' from %4 [ProviderID: %5]")
                         .arg(rootCard.getName())
                         .arg(rootCard.getPrinting().getSet()->getShortName())
                         .arg(rootCard.getPrinting().getProperty("num"))
                         .arg(zone == DECK_ZONE_MAIN ? "mainboard" : "sideboard")
                         .arg(rootCard.getPrinting().getUuid());

    emit deckModified(reason);

    QModelIndex idx = deckModel->findCard(rootCard.getName(), zone, rootCard.getPrinting().getUuid(),
                                          rootCard.getPrinting().getProperty("num"));

    deckModel->decrementAmountAtIndex(idx);
    deckEditor->setModified(true);
}

/**
 * @brief Counts the number of cards in a specific zone (mainboard or sideboard).
 *
 * @param deckZone The name of the zone (e.g., DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 * @return The number of cards in the zone.
 */
int CardAmountWidget::countCardsInZone(const QString &deckZone)
{
    if (rootCard.getPrinting().getUuid().isEmpty()) {
        return 0; // Cards without uuids/providerIds CANNOT match another card, they are undefined for us.
    }

    if (!deckModel) {
        return -1;
    }

    QList<ExactCard> cards = deckModel->getCardsForZone(deckZone);

    int count = 0;
    for (auto currentCard : cards) {
        if (currentCard.getPrinting().getUuid() == rootCard.getPrinting().getProperty("uuid")) {
            count++;
        }
    }

    return count;
}