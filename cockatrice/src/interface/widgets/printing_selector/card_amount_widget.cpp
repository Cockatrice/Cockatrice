#include "card_amount_widget.h"

#include "../deck_editor/deck_state_manager.h"

#include <QPainter>
#include <QTimer>

/**
 * @brief Constructs a widget for displaying and controlling the card count in a specific zone.
 *
 * @param parent The parent widget.
 * @param cardSizeSlider Pointer to the QSlider for adjusting font size.
 * @param rootCard The root card to manage within the widget.
 * @param zoneName The zone name (e.g., DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
CardAmountWidget::CardAmountWidget(QWidget *parent,
                                   DeckStateManager *deckStateManager,
                                   QSlider *cardSizeSlider,
                                   const ExactCard &rootCard,
                                   const QString &zoneName)
    : QWidget(parent), deckStateManager(deckStateManager), cardSizeSlider(cardSizeSlider), rootCard(rootCard),
      zoneName(zoneName), hovered(false)
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

    cardCountInZone = new QLabel(QString::number(amount), this);
    cardCountInZone->setAlignment(Qt::AlignCenter);

    layout->addWidget(decrementButton);
    layout->addWidget(cardCountInZone);
    layout->addWidget(incrementButton);

    // Connect slider for dynamic font size adjustment
    connect(cardSizeSlider, &QSlider::valueChanged, this, &CardAmountWidget::adjustFontSize);
}

int CardAmountWidget::getAmount()
{
    return amount;
}

void CardAmountWidget::setAmount(int _amount)
{
    amount = _amount;
    updateCardCount();
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
    cardCountInZone->setText("<font color='white'>" + QString::number(amount) + "</font>");
    layout->invalidate();
    layout->activate();
}

static QModelIndex addAndReplacePrintings(DeckListModel *model,
                                          const QModelIndex &existing,
                                          const ExactCard &rootCard,
                                          const QString &zone,
                                          int extraCopies,
                                          bool replaceProviderless)
{
    auto newCardIndex = model->addCard(rootCard, zone);
    if (!newCardIndex.isValid()) {
        return {};
    }

    // Check if a card without a providerId already exists in the deckModel and replace it, if so.
    if (existing.isValid() && existing != newCardIndex && replaceProviderless) {
        model->offsetCountAtIndex(newCardIndex, extraCopies);
        model->removeRow(existing.row(), existing.parent());
    }

    // Set Index and Focus as if the user had just clicked the new card and modify the deckEditor saveState
    return model->findCard(rootCard.getName(), zone, rootCard.getPrinting().getUuid(),
                           rootCard.getPrinting().getProperty("num"));
}

/**
 * @brief Adds a printing of the card to the specified zone (Mainboard or Sideboard).
 *
 * @param zone The zone to add the card to (DECK_ZONE_MAIN or DECK_ZONE_SIDE).
 */
void CardAmountWidget::addPrinting(const QString &zone)
{
    // Check if we will need to add extra copies due to replacing copies without providerIds
    QModelIndex existing = deckStateManager->getModel()->findCard(rootCard.getName(), zone);

    int extraCopies = 0;
    bool replacingProviderless = false;

    if (existing.isValid()) {
        QString foundProviderId =
            existing.siblingAtColumn(DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::DisplayRole).toString();
        if (foundProviderId.isEmpty()) {
            int existingAmount = existing.data(Qt::DisplayRole).toInt();
            extraCopies = existingAmount - 1; // One less because we *always* add one
            replacingProviderless = true;
        }
    }

    QString reason = QString("Added %1 copies of '%2 (%3) %4' to %5 [ProviderID: %6]%7")
                         .arg(1 + extraCopies)
                         .arg(rootCard.getName())
                         .arg(rootCard.getPrinting().getSet()->getShortName())
                         .arg(rootCard.getPrinting().getProperty("num"))
                         .arg(zone == DECK_ZONE_MAIN ? "mainboard" : "sideboard")
                         .arg(rootCard.getPrinting().getUuid())
                         .arg(replacingProviderless ? " (replaced providerless printings)" : "");

    // Add the card and expand the list UI
    QModelIndex newCardIndex = deckStateManager->modifyDeck(reason, [&](auto model) {
        return addAndReplacePrintings(model, existing, rootCard, zone, extraCopies, replacingProviderless);
    });

    if (newCardIndex.isValid()) {
        emit deckStateManager->focusIndexChanged(newCardIndex);
    }
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

    deckStateManager->modifyDeck(reason, [this, &zone](auto model) {
        QModelIndex idx = model->findCard(rootCard.getName(), zone, rootCard.getPrinting().getUuid(),
                                          rootCard.getPrinting().getProperty("num"));
        return model->offsetCountAtIndex(idx, -1);
    });
}
