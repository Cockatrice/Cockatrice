#include "dlg_select_set_for_cards.h"

#include "../../deck_loader/deck_loader.h"
#include "../interface/widgets/cards/card_info_picture_widget.h"
#include "../interface/widgets/general/layout_containers/flow_widget.h"
#include "dlg_select_set_for_cards.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <algorithm>
#include <libcockatrice/card/database/card_database_manager.h>
#include <qdrag.h>
#include <qevent.h>

DlgSelectSetForCards::DlgSelectSetForCards(QWidget *parent, DeckListModel *_model) : QDialog(parent), model(_model)
{
    setMinimumSize(500, 500);
    setAcceptDrops(true);

    instructionLabel = new QLabel(this);
    instructionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    // Main vertical splitter
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // Top scroll area
    scrollArea = new QScrollArea(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);

    listContainer = new QWidget(scrollArea);
    listLayout = new QVBoxLayout(listContainer);
    listContainer->setLayout(listLayout);
    scrollArea->setWidget(listContainer);

    // Bottom horizontal splitter
    QSplitter *bottomSplitter = new QSplitter(Qt::Horizontal, this);

    // Left container
    QWidget *leftContainer = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    uneditedCardsLabel = new QLabel(this);
    uneditedCardsArea = new QScrollArea(this);
    uneditedCardsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    uneditedCardsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    uneditedCardsArea->setWidgetResizable(true);

    uneditedCardsFlowWidget =
        new FlowWidget(uneditedCardsArea, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    uneditedCardsArea->setWidget(uneditedCardsFlowWidget);

    leftLayout->addWidget(uneditedCardsLabel);
    leftLayout->addWidget(uneditedCardsArea);
    leftContainer->setLayout(leftLayout);

    // Right container
    QWidget *rightContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    modifiedCardsLabel = new QLabel(this);
    modifiedCardsArea = new QScrollArea(this);
    modifiedCardsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    modifiedCardsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    modifiedCardsArea->setWidgetResizable(true);

    modifiedCardsFlowWidget =
        new FlowWidget(modifiedCardsArea, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    modifiedCardsArea->setWidget(modifiedCardsFlowWidget);

    rightLayout->addWidget(modifiedCardsLabel);
    rightLayout->addWidget(modifiedCardsArea);
    rightContainer->setLayout(rightLayout);

    // Add left and right containers to the bottom splitter
    bottomSplitter->addWidget(leftContainer);
    bottomSplitter->addWidget(rightContainer);

    // Add widgets to the main splitter
    splitter->addWidget(scrollArea);
    splitter->addWidget(bottomSplitter);

    cardsForSets = getCardsForSets();

    sortSetsByCount();
    updateCardLists();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    clearButton = new QPushButton(buttonBox);
    connect(clearButton, &QPushButton::clicked, this, &DlgSelectSetForCards::actClear);

    setAllToPreferredButton = new QPushButton(buttonBox);
    connect(setAllToPreferredButton, &QPushButton::clicked, this, &DlgSelectSetForCards::actSetAllToPreferred);

    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(setAllToPreferredButton, QDialogButtonBox::ActionRole);

    // Set stretch factors
    splitter->setStretchFactor(0, 6); // Scroll area gets more space
    splitter->setStretchFactor(1, 2); // Bottom part gets less space
    splitter->setStretchFactor(2, 1); // Buttons take minimal space

    bottomSplitter->setStretchFactor(0, 1); // Left and right equally split
    bottomSplitter->setStretchFactor(1, 1);

    connect(this, &DlgSelectSetForCards::orderChanged, this, &DlgSelectSetForCards::updateLayoutOrder);
    connect(this, &DlgSelectSetForCards::widgetOrderChanged, this, &DlgSelectSetForCards::updateCardLists);

    mainLayout->addWidget(instructionLabel);
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(buttonBox);

    retranslateUi();
    setWindowFlags(Qt::Window);
    showMaximized();
}

void DlgSelectSetForCards::retranslateUi()
{
    uneditedCardsLabel->setText(tr("Unmodified Cards:"));
    modifiedCardsLabel->setText(tr("Modified Cards:"));
    instructionLabel->setText(tr("Check Sets to enable them. Drag-and-Drop to reorder them and change their "
                                 "priority. Cards will use the printing of the highest priority enabled set."));
    clearButton->setText(tr("Clear all set information"));
    setAllToPreferredButton->setText(tr("Set all to preferred"));
}

void DlgSelectSetForCards::actOK()
{
    QMap<QString, QStringList> modifiedSetsAndCardsMap = getModifiedCards();
    for (QString modifiedSet : modifiedSetsAndCardsMap.keys()) {
        for (QString card : modifiedSetsAndCardsMap.value(modifiedSet)) {
            QModelIndex find_card = model->findCard(card, DECK_ZONE_MAIN);
            if (!find_card.isValid()) {
                continue;
            }
            model->removeRow(find_card.row(), find_card.parent());
            CardInfoPtr cardInfo = CardDatabaseManager::query()->getCardInfo(card);
            PrintingInfo printing = CardDatabaseManager::query()->getSpecificPrinting(card, modifiedSet, "");
            model->addCard(ExactCard(cardInfo, printing), DECK_ZONE_MAIN);
        }
    }
    accept();
}

void DlgSelectSetForCards::actClear()
{
    qobject_cast<DeckLoader *>(model->getDeckList())->clearSetNamesAndNumbers();
    accept();
}

void DlgSelectSetForCards::actSetAllToPreferred()
{
    qobject_cast<DeckLoader *>(model->getDeckList())->clearSetNamesAndNumbers();
    qobject_cast<DeckLoader *>(model->getDeckList())->setProviderIdToPreferredPrinting();
    accept();
}

void DlgSelectSetForCards::sortSetsByCount()
{
    QMap<QString, int> setsForCards = getSetsForCards();

    // Convert map to a sortable list
    QVector<QPair<QString, int>> setList;
    for (auto it = setsForCards.begin(); it != setsForCards.end(); ++it) {
        setList.append(qMakePair(it.key(), it.value()));
    }

    // Sort in descending order of count
    std::sort(setList.begin(), setList.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) { return a.second > b.second; });

    // Clear existing entries
    qDeleteAll(setEntries);
    setEntries.clear();

    // Populate with sorted entries
    for (const auto &entry : setList) {
        SetEntryWidget *widget = new SetEntryWidget(this, entry.first, entry.second);
        listLayout->addWidget(widget);
        setEntries.insert(entry.first, widget);
    }
}

QMap<QString, int> DlgSelectSetForCards::getSetsForCards()
{
    QMap<QString, int> setCounts;
    if (!model)
        return setCounts;

    DeckList *decklist = model->getDeckList();
    if (!decklist)
        return setCounts;

    QList<DecklistCardNode *> cardsInDeck = decklist->getCardNodes();

    for (auto currentCard : cardsInDeck) {
        CardInfoPtr infoPtr = CardDatabaseManager::query()->getCardInfo(currentCard->getName());
        if (!infoPtr)
            continue;

        SetToPrintingsMap setMap = infoPtr->getSets();
        for (auto &setName : setMap.keys()) {
            setCounts[setName]++;
        }
    }

    return setCounts;
}

void DlgSelectSetForCards::updateCardLists()
{
    for (SetEntryWidget *entryWidget : entry_widgets) {
        entryWidget->populateCardList();
        if (entryWidget->expanded) {
            entryWidget->updateCardDisplayWidgets();
        }
        entryWidget->checkVisibility();
    }

    uneditedCardsFlowWidget->clearLayout();
    modifiedCardsFlowWidget->clearLayout();

    // Map from set name to a set of selected cards in that set
    QMap<QString, QSet<QString>> selectedCardsBySet;
    for (SetEntryWidget *entryWidget : entry_widgets) {
        if (entryWidget->isChecked()) {
            QStringList cardsInSet = entryWidget->getAllCardsForSet();
            QSet<QString> cardSet = QSet<QString>(cardsInSet.begin(), cardsInSet.end()); // Convert list to set
            selectedCardsBySet.insert(entryWidget->setName, cardSet);
        }
    }

    DeckList *decklist = model->getDeckList();
    if (!decklist)
        return;

    QList<DecklistCardNode *> cardsInDeck = decklist->getCardNodes();

    for (auto currentCard : cardsInDeck) {
        bool found = false;
        QString foundSetName;

        // Check across all sets if the card is present
        for (auto it = selectedCardsBySet.begin(); it != selectedCardsBySet.end(); ++it) {
            if (it.value().contains(currentCard->getName())) {
                found = true;
                foundSetName = it.key(); // Store the set name where it was found
                break;                   // Stop at the first match
            }
        }

        if (!found) {
            // The card was not in any selected set
            ExactCard card = CardDatabaseManager::query()->getCard({currentCard->getName()});
            CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(uneditedCardsFlowWidget);
            picture_widget->setCard(card);
            uneditedCardsFlowWidget->addWidget(picture_widget);
        } else {
            ExactCard card = CardDatabaseManager::query()->getCard(
                {currentCard->getName(), CardDatabaseManager::getInstance()
                                             ->query()
                                             ->getSpecificPrinting(currentCard->getName(), foundSetName, "")
                                             .getUuid()});
            CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(modifiedCardsFlowWidget);
            picture_widget->setCard(card);
            modifiedCardsFlowWidget->addWidget(picture_widget);
        }
    }
}

void DlgSelectSetForCards::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-setentrywidget")) {
        // Highlight the drop target area
        event->acceptProposedAction();
        // Optionally, change cursor to indicate a valid drop area
        setCursor(Qt::OpenHandCursor);
    }
}

void DlgSelectSetForCards::dropEvent(QDropEvent *event)
{
    QByteArray itemData = event->mimeData()->data("application/x-setentrywidget");
    QString draggedSetName = QString::fromUtf8(itemData);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint adjustedPos = event->position().toPoint() + QPoint(0, scrollArea->verticalScrollBar()->value());
#else
    QPoint adjustedPos = event->pos() + QPoint(0, scrollArea->verticalScrollBar()->value());
#endif
    int dropIndex = -1;
    for (int i = 0; i < listLayout->count(); ++i) {
        QWidget *widget = listLayout->itemAt(i)->widget();
        if (widget && widget->geometry().contains(adjustedPos)) {
            dropIndex = i;
            break;
        }
    }

    if (dropIndex != -1) {
        // Find the dragged widget and move it to the new position
        SetEntryWidget *draggedWidget = setEntries.value(draggedSetName, nullptr);
        if (draggedWidget) {
            listLayout->removeWidget(draggedWidget);
            listLayout->insertWidget(dropIndex, draggedWidget);
        }
    }

    event->acceptProposedAction();
    // Reset cursor after drop
    unsetCursor();

    // We need to execute this AFTER the current event-cycle so we use a timer.
    QTimer::singleShot(10, this, [this]() { emit orderChanged(); });
}

QMap<QString, QStringList> DlgSelectSetForCards::getCardsForSets()
{
    QMap<QString, QStringList> setCards;
    if (!model)
        return setCards;

    DeckList *decklist = model->getDeckList();
    if (!decklist)
        return setCards;

    QList<DecklistCardNode *> cardsInDeck = decklist->getCardNodes();

    for (auto currentCard : cardsInDeck) {
        CardInfoPtr infoPtr = CardDatabaseManager::query()->getCardInfo(currentCard->getName());
        if (!infoPtr)
            continue;

        SetToPrintingsMap setMap = infoPtr->getSets();
        for (auto it = setMap.begin(); it != setMap.end(); ++it) {
            setCards[it.key()].append(currentCard->getName());
        }
    }

    return setCards;
}

QMap<QString, QStringList> DlgSelectSetForCards::getModifiedCards()
{
    QMap<QString, QStringList> modifiedCards;
    for (int i = 0; i < listLayout->count(); ++i) {
        QWidget *widget = listLayout->itemAt(i)->widget();
        if (auto entry = qobject_cast<SetEntryWidget *>(widget)) {
            if (entry->isChecked()) {
                QStringList cardsInSet = entry->getAllCardsForSet();

                for (QString cardInSet : cardsInSet) {
                    bool alreadyContained = false;
                    for (QString key : modifiedCards.keys()) {
                        if (modifiedCards[key].contains(cardInSet)) {
                            alreadyContained = true;
                        }
                    }
                    if (!alreadyContained) {
                        modifiedCards[entry->setName].append(cardInSet);
                    }
                }
            }
        }
    }
    return modifiedCards;
}

void DlgSelectSetForCards::updateLayoutOrder()
{
    entry_widgets.clear();
    for (int i = 0; i < listLayout->count(); ++i) {
        QWidget *widget = listLayout->itemAt(i)->widget();
        if (auto entry = qobject_cast<SetEntryWidget *>(widget)) {
            entry_widgets.append(entry);
        }
    }

    emit widgetOrderChanged();
}

SetEntryWidget::SetEntryWidget(DlgSelectSetForCards *_parent, const QString &_setName, int count)
    : QWidget(_parent), parent(_parent), setName(_setName), expanded(false)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    CardSetPtr set = CardDatabaseManager::getInstance()->getSet(setName);
    checkBox = new QCheckBox("(" + set->getShortName() + ") - " + set->getLongName(), this);
    connect(checkBox, &QCheckBox::toggled, parent, &DlgSelectSetForCards::updateLayoutOrder);
    expandButton = new QPushButton("+", this);
    countLabel = new QLabel(QString::number(count), this);

    connect(expandButton, &QPushButton::clicked, this, &SetEntryWidget::toggleExpansion);

    headerLayout->addWidget(checkBox);
    headerLayout->addWidget(countLabel);
    headerLayout->addWidget(expandButton);
    layout->addLayout(headerLayout);

    possibleCardsLabel = new QLabel(this);
    possibleCardsLabel->setText("Unselected cards in set:");
    possibleCardsLabel->hide();

    cardListContainer = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    cardListContainer->hide();

    alreadySelectedCardsLabel = new QLabel(this);
    alreadySelectedCardsLabel->setText("Cards in set already selected in higher priority set:");
    alreadySelectedCardsLabel->hide();

    alreadySelectedCardListContainer =
        new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    alreadySelectedCardListContainer->hide();

    layout->addWidget(possibleCardsLabel);
    layout->addWidget(cardListContainer);
    layout->addWidget(alreadySelectedCardsLabel);
    layout->addWidget(alreadySelectedCardListContainer);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_StyledBackground, true);
}

void SetEntryWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Create a drag object and set up mime data
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        // Set the mime data to store the dragged set's name
        mimeData->setData("application/x-setentrywidget", setName.toUtf8());
        drag->setMimeData(mimeData);

        // Create a "ghost" pixmap to represent the widget during dragging
        QPixmap pixmap = this->grab();

        // Ensure pixmap has a transparent background
        QImage image = pixmap.toImage();
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (image.pixel(x, y) == Qt::transparent) {
                    image.setPixel(x, y, QColor(0, 0, 0, 0).rgba()); // Set transparency where needed
                }
            }
        }

        pixmap = QPixmap::fromImage(image); // Convert back to pixmap after transparency manipulation

        // Set the pixmap for the drag object
        drag->setPixmap(pixmap);

        // Optionally adjust the pixmap position offset to align with the cursor
        drag->setHotSpot(event->pos());

        drag->exec(Qt::MoveAction);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SetEntryWidget::enterEvent(QEnterEvent *event)
#else
void SetEntryWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event); // Call the base class handler
    // Highlight the widget by changing the background color only for the widget itself
    setStyleSheet("SetEntryWidget { background: gray; }");

    // Change cursor to open hand
    setCursor(Qt::OpenHandCursor);
    repaint();
}

void SetEntryWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event); // Call the base class handler
    // Reset the background color only for the widget itself
    setStyleSheet("SetEntryWidget { background: none; }");

    // Reset cursor to default
    setCursor(Qt::ArrowCursor);
    repaint();
}

void SetEntryWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // Check if the mime data is of the correct type
    if (event->mimeData()->hasFormat("application/x-setentrywidget")) {
        setCursor(Qt::ClosedHandCursor); // Hand cursor to indicate move
        // Get the current position of the widget being dragged

        // For now, we will just highlight the widget when dragged.
        QPainter painter(this);
        QColor highlightColor(255, 255, 255, 128); // Semi-transparent white
        painter.setBrush(QBrush(highlightColor));
        painter.setPen(Qt::NoPen);
        painter.drawRect(this->rect()); // Highlight the widget area

        // Allow the widget to be moved to the new position
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

bool SetEntryWidget::isChecked() const
{
    return checkBox->isChecked();
}

void SetEntryWidget::toggleExpansion()
{
    expanded = !expanded;
    possibleCardsLabel->setVisible(expanded);
    cardListContainer->setVisible(expanded);
    alreadySelectedCardsLabel->setVisible(expanded);
    alreadySelectedCardListContainer->setVisible(expanded);
    expandButton->setText(expanded ? "-" : "+");

    populateCardList();
    updateCardDisplayWidgets();

    parent->updateCardLists();
}

void SetEntryWidget::checkVisibility()
{
    if (possibleCards.empty()) {
        setHidden(true);
    } else {
        setVisible(true);
    }
}

QStringList SetEntryWidget::getAllCardsForSet()
{
    QStringList list;
    QMap<QString, QStringList> setCards = parent->cardsForSets;
    if (setCards.contains(setName)) {
        for (const QString &cardName : setCards[setName]) {
            list << cardName;
        }
    }
    return list;
}

void SetEntryWidget::populateCardList()
{
    possibleCards = getAllCardsForSet();

    for (SetEntryWidget *entryWidget : parent->entry_widgets) {
        if (entryWidget == this) {
            break;
        }
        if (entryWidget->isChecked()) {
            for (const QString &cardName : entryWidget->possibleCards) {
                possibleCards.removeAll(cardName);
            }
        }
    }

    unusedCards = getAllCardsForSet();
    for (const QString &cardName : possibleCards) {
        unusedCards.removeAll(cardName);
    }
    checkVisibility();
    countLabel->setText(QString::number(possibleCards.size()) + " (" +
                        QString::number(possibleCards.size() + unusedCards.size()) + ")");
}

void SetEntryWidget::updateCardDisplayWidgets()
{
    cardListContainer->clearLayout();
    alreadySelectedCardListContainer->clearLayout();

    for (const QString &cardName : possibleCards) {
        CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(cardListContainer);
        QString providerId = CardDatabaseManager::query()->getSpecificPrinting(cardName, setName, nullptr).getUuid();
        picture_widget->setCard(CardDatabaseManager::query()->getCard({cardName, providerId}));
        cardListContainer->addWidget(picture_widget);
    }

    for (const QString &cardName : unusedCards) {
        CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(alreadySelectedCardListContainer);
        QString providerId = CardDatabaseManager::query()->getSpecificPrinting(cardName, setName, nullptr).getUuid();
        picture_widget->setCard(CardDatabaseManager::query()->getCard({cardName, providerId}));
        alreadySelectedCardListContainer->addWidget(picture_widget);
    }
}
