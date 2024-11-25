#include "card_amount_widget.h"

CardAmountWidget::CardAmountWidget(QWidget *parent,
                                   TabDeckEditor *deckEditor,
                                   DeckListModel *deckModel,
                                   QTreeView *deckView,
                                   CardInfoPtr &rootCard,
                                   CardInfoPerSet &setInfoForCard,
                                   QString zoneName)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), rootCard(rootCard),
      setInfoForCard(setInfoForCard), zoneName(zoneName), hovered(false)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    incrementButton = new QPushButton(this);
    incrementButton->setText("+");
    decrementButton = new QPushButton(this);
    decrementButton->setText("-");

    incrementButton->setFixedSize(30, 30);
    decrementButton->setFixedSize(30, 30);

    // Apply styles for gradient buttons
    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,
                stop:0 rgba(64, 64, 64, 255), stop:1 rgba(32, 32, 32, 255));
            border: none;
            color: white;
            font-size: 16px;
        }
        QPushButton:hover {
            background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,
                stop:0 rgba(96, 96, 96, 255), stop:1 rgba(48, 48, 48, 255));
        }
        QPushButton:pressed {
            background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,
                stop:0 rgba(128, 128, 128, 255), stop:1 rgba(64, 64, 64, 255));
        }
    )";
    incrementButton->setStyleSheet(buttonStyle);
    decrementButton->setStyleSheet(buttonStyle);

    // Set up connections
    if (zoneName == DECK_ZONE_MAIN) {
        connect(incrementButton, &QPushButton::clicked, this, &CardAmountWidget::addPrintingMainboard);
        connect(decrementButton, &QPushButton::clicked, this, &CardAmountWidget::removePrintingMainboard);
    } else if (zoneName == DECK_ZONE_SIDE) {
        connect(incrementButton, &QPushButton::clicked, this, &CardAmountWidget::addPrintingSideboard);
        connect(decrementButton, &QPushButton::clicked, this, &CardAmountWidget::removePrintingSideboard);
    }

    cardCountInZone = new QLabel(QString::number(countCardsInZone(zoneName)), this);
    cardCountInZone->setStyleSheet("color: white; font-size: 16px;");
    cardCountInZone->setAlignment(Qt::AlignCenter);

    layout->addWidget(decrementButton);
    layout->addWidget(cardCountInZone);
    layout->addWidget(incrementButton);

    // React to model changes
    connect(deckModel, &DeckListModel::dataChanged, this, &CardAmountWidget::updateCardCount);
    connect(deckModel, &QAbstractItemModel::rowsRemoved, this, &CardAmountWidget::updateCardCount);
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

void CardAmountWidget::updateCardCount()
{
    cardCountInZone->setText(QString::number(countCardsInZone(zoneName)));
}

void CardAmountWidget::addPrinting(const QString &zone)
{
    const auto newCardIndex = deckModel->addCard(rootCard->getName(), setInfoForCard, zone);
    recursiveExpand(newCardIndex);
    QModelIndex find_card = deckModel->findCard(rootCard->getName(), zone);
    if (find_card.isValid() && find_card != newCardIndex) {
        deckModel->removeRow(find_card.row(), find_card.parent());
    };
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
    idx = deckModel->findCard(rootCard->getName(), zone, setInfoForCard.getProperty("uuid"));
    offsetCountAtIndex(idx, -1);
}

int CardAmountWidget::countCardsInZone(const QString &deckZone)
{
    int count = 0;

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