#include "card_amount_widget.h"

#include "../general/display/dynamic_font_size_push_button.h"

#include <QTimer>

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
    incrementButton->setText("+");
    decrementButton = new DynamicFontSizePushButton(this);
    decrementButton->setText("-");

    incrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
    decrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);

    // Set up connections
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
    adjustFontSize(cardSizeSlider->value());

    // Resize after a little delay since, initially, the parent widget has no size.
    QTimer::singleShot(50, this, [this]() {
        incrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
        decrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
    });
}

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

void CardAmountWidget::adjustFontSize(int scalePercentage)
{
    qDebug() << scalePercentage;
    const int minFontSize = 8;      // Minimum font size
    const int maxFontSize = 32;     // Maximum font size
    const int basePercentage = 100; // Scale at 100%

    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / (250 - 25);
    newFontSize = std::clamp(newFontSize, minFontSize, maxFontSize);

    qDebug() << newFontSize;

    // Update the font for card count label
    QFont cardCountFont = cardCountInZone->font();
    cardCountFont.setPointSize(newFontSize);
    cardCountInZone->setFont(cardCountFont);

    incrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);
    decrementButton->setFixedSize(parentWidget()->size().width() / 3, parentWidget()->size().height() / 9);

    // Repaint the widget (if necessary)
    repaint();
}

void CardAmountWidget::updateCardCount()
{
    cardCountInZone->setText("<font color='white'>" + QString::number(countCardsInZone(zoneName)) + "</font>");
    layout->invalidate();
    layout->activate();
}

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
    newCardIndex = deckModel->findCard(rootCard->getName(), zone, setInfoForCard.getProperty("uuid"));
    deckView->setCurrentIndex(newCardIndex);
    deckView->setFocus(Qt::FocusReason::MouseFocusReason);
}

void CardAmountWidget::addPrintingMainboard()
{
    addPrinting(DECK_ZONE_MAIN);
}

void CardAmountWidget::addPrintingSideboard()
{
    addPrinting(DECK_ZONE_SIDE);
}

void CardAmountWidget::removePrintingMainboard()
{
    decrementCardHelper(DECK_ZONE_MAIN);
}

void CardAmountWidget::removePrintingSideboard()
{
    decrementCardHelper(DECK_ZONE_SIDE);
}

void CardAmountWidget::recursiveExpand(const QModelIndex &index)
{
    if (index.parent().isValid()) {
        recursiveExpand(index.parent());
    }
    deckView->expand(index);
}

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

void CardAmountWidget::decrementCardHelper(const QString &zone)
{
    QModelIndex idx;
    idx = deckModel->findCard(rootCard->getName(), zone, setInfoForCard.getProperty("uuid"), setInfoForCard.getProperty("num"));
    offsetCountAtIndex(idx, -1);
}

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