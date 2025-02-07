#include "dlg_select_set_for_cards.h"

#include "../client/ui/widgets/cards/card_info_picture_widget.h"
#include "../client/ui/widgets/general/layout_containers/flow_widget.h"
#include "../deck/deck_loader.h"
#include "../game/cards/card_database_manager.h"
#include "dlg_select_set_for_cards.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <algorithm>
#include <qdrag.h>
#include <qevent.h>

DlgSelectSetForCards::DlgSelectSetForCards(QWidget *parent, DeckListModel *_model) : QDialog(parent), model(_model)
{
    setMinimumSize(500, 500);
    setAcceptDrops(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    mainLayout->addWidget(splitter);

    // Top scroll area
    scrollArea = new QScrollArea(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);

    listContainer = new QWidget(scrollArea);
    listLayout = new QVBoxLayout(listContainer);
    listContainer->setLayout(listLayout);
    scrollArea->setWidget(listContainer);

    // Bottom section container
    QWidget *bottomContainer = new QWidget(this);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomContainer);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    uneditedCardsLabel = new QLabel("Unmodified Cards:", this);
    uneditedCardsArea = new QScrollArea(this);
    uneditedCardsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    uneditedCardsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    uneditedCardsArea->setWidgetResizable(true);

    uneditedCardsFlowWidget =
        new FlowWidget(uneditedCardsArea, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    uneditedCardsArea->setWidget(uneditedCardsFlowWidget);

    bottomLayout->addWidget(uneditedCardsLabel);
    bottomLayout->addWidget(uneditedCardsArea);
    bottomContainer->setLayout(bottomLayout);

    // Add widgets to the splitter
    splitter->addWidget(scrollArea);
    splitter->addWidget(bottomContainer);

    sortSetsByCount();
    updateCardLists();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    splitter->addWidget(buttonBox);

    // Set stretch factors: top (2:3), bottom (1:3)
    splitter->setStretchFactor(0, 6);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 1);
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
            model->addCard(card, CardDatabaseManager::getInstance()->getSpecificSetForCard(card, modifiedSet, ""),
                           DECK_ZONE_MAIN);
        }
    }
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

    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return setCounts;

    for (auto *i : *listRoot) {
        auto *countCurrentZone = dynamic_cast<InnerDecklistNode *>(i);
        if (!countCurrentZone)
            continue;

        for (auto *cardNode : *countCurrentZone) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(cardNode);
            if (!currentCard)
                continue;

            CardInfoPtr infoPtr = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
            if (!infoPtr)
                continue;

            CardInfoPerSetMap infoPerSetMap = infoPtr->getSets();
            for (auto it = infoPerSetMap.begin(); it != infoPerSetMap.end(); ++it) {
                setCounts[it.key()]++;
            }
        }
    }
    return setCounts;
}

void DlgSelectSetForCards::updateCardLists()
{
    QList<SetEntryWidget *> entry_widgets;
    for (int i = 0; i < listLayout->count(); ++i) {
        QWidget *widget = listLayout->itemAt(i)->widget();
        if (auto entry = qobject_cast<SetEntryWidget *>(widget)) {
            entry_widgets.append(entry);
        }
    }
    for (SetEntryWidget *entryWidget : entry_widgets) {
        if (entryWidget->expanded) {
            entryWidget->populateCardList();
        }
    }

    uneditedCardsFlowWidget->clearLayout();

    QStringList selectedCards;
    for (SetEntryWidget *entryWidget : entry_widgets) {
        if (entryWidget->isChecked()) {
            QStringList cardsInSet = entryWidget->getAllCardsForSet();
            for (QString cardInSet : cardsInSet) {
                selectedCards.append(cardInSet);
            }
        }
    }
    selectedCards.removeDuplicates();

    DeckList *decklist = model->getDeckList();
    if (!decklist)
        return;

    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return;

    for (auto *i : *listRoot) {
        auto *countCurrentZone = dynamic_cast<InnerDecklistNode *>(i);
        if (!countCurrentZone)
            continue;

        for (auto *cardNode : *countCurrentZone) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(cardNode);
            if (!currentCard)
                continue;

            if (!selectedCards.contains(currentCard->getName())) {
                CardInfoPtr infoPtr = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
                CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(uneditedCardsFlowWidget);
                picture_widget->setCard(infoPtr);
                uneditedCardsFlowWidget->addWidget(picture_widget);
            }
        }
    }
}

void DlgSelectSetForCards::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-setentrywidget")) {
        event->acceptProposedAction();
    }
}

void DlgSelectSetForCards::dropEvent(QDropEvent *event)
{
    QByteArray itemData = event->mimeData()->data("application/x-setentrywidget");
    QString draggedSetName = QString::fromUtf8(itemData);

    int dropIndex = -1;
    for (int i = 0; i < listLayout->count(); ++i) {
        QWidget *widget = listLayout->itemAt(i)->widget();
        if (widget && widget->geometry().contains(event->position().toPoint())) {
            dropIndex = i;
            break;
        }
    }

    if (dropIndex != -1) {
        // Find the dragged widget
        SetEntryWidget *draggedWidget = setEntries.value(draggedSetName, nullptr);
        if (draggedWidget) {
            listLayout->removeWidget(draggedWidget);
            listLayout->insertWidget(dropIndex, draggedWidget);
        }
    }

    event->acceptProposedAction();
}

QMap<QString, QStringList> DlgSelectSetForCards::getCardsForSets()
{
    QMap<QString, QStringList> setCards;
    if (!model)
        return setCards;

    DeckList *decklist = model->getDeckList();
    if (!decklist)
        return setCards;

    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return setCards;

    for (auto *i : *listRoot) {
        auto *countCurrentZone = dynamic_cast<InnerDecklistNode *>(i);
        if (!countCurrentZone)
            continue;

        for (auto *cardNode : *countCurrentZone) {
            auto *currentCard = dynamic_cast<DecklistCardNode *>(cardNode);
            if (!currentCard)
                continue;

            CardInfoPtr infoPtr = CardDatabaseManager::getInstance()->getCard(currentCard->getName());
            if (!infoPtr)
                continue;

            CardInfoPerSetMap infoPerSetMap = infoPtr->getSets();
            for (auto it = infoPerSetMap.begin(); it != infoPerSetMap.end(); ++it) {
                setCards[it.key()].append(currentCard->getName());
            }
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

SetEntryWidget::SetEntryWidget(DlgSelectSetForCards *_parent, const QString &_setName, int count)
    : QWidget(_parent), parent(_parent), setName(_setName), expanded(false)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    checkBox = new QCheckBox(setName, this);
    connect(checkBox, &QCheckBox::checkStateChanged, parent, &DlgSelectSetForCards::updateCardLists);
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
}

void SetEntryWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setData("application/x-setentrywidget", setName.toUtf8());
        drag->setMimeData(mimeData);

        // Create a drag preview (snapshot of the widget)
        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent); // Ensure transparency

        QPainter painter(&pixmap);
        this->render(&painter);
        painter.end();

        drag->setPixmap(pixmap);
        drag->setHotSpot(event->position().toPoint()); // Keeps the cursor aligned

        drag->exec(Qt::MoveAction);
    }
}

void SetEntryWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    // Start a drag operation
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData();
    mimeData->setText("SetEntryWidget Data"); // Customize with relevant data
    drag->setMimeData(mimeData);

    // Create a drag preview of the widget
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);

    // Render the widget onto the pixmap
    render(&pixmap);

    // Optionally, apply transparency to make it look like a drag effect
    QPixmap transparentPixmap = pixmap;
    QPainter painter(&transparentPixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.fillRect(transparentPixmap.rect(), QColor(0, 0, 0, 128)); // Semi-transparent effect
    painter.end();

    // Set the drag pixmap
    drag->setPixmap(transparentPixmap);
    drag->setHotSpot(event->pos()); // Ensure the drag starts from where the user clicked

    // Start the drag operation
    drag->exec(Qt::MoveAction);
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

    parent->updateCardLists();
}

QStringList SetEntryWidget::getAllCardsForSet()
{
    QStringList list;
    QMap<QString, QStringList> setCards = parent->getCardsForSets();
    if (setCards.contains(setName)) {
        for (const QString &cardName : setCards[setName]) {
            list << cardName;
        }
    }
    return list;
}

void SetEntryWidget::populateCardList()
{
    cardListContainer->clearLayout();
    alreadySelectedCardListContainer->clearLayout();

    QStringList possibleCards = getAllCardsForSet();
    QList<SetEntryWidget *> entry_widgets;
    for (int i = 0; i < parent->listLayout->count(); ++i) {
        QWidget *widget = parent->listLayout->itemAt(i)->widget();
        if (auto entry = qobject_cast<SetEntryWidget *>(widget)) {
            entry_widgets.append(entry);
        }
    }

    for (SetEntryWidget *entryWidget : entry_widgets) {
        if (entryWidget == this) {
            break;
        }
        if (entryWidget->isChecked()) {
            QStringList alreadyDoneCards = entryWidget->getAllCardsForSet();
            for (const QString &cardName : alreadyDoneCards) {
                possibleCards.removeAll(cardName);
            }
        }
    }

    for (const QString &cardName : possibleCards) {
        CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(cardListContainer);
        picture_widget->setCard(CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
            cardName,
            CardDatabaseManager::getInstance()->getSpecificSetForCard(cardName, setName, nullptr).getProperty("uuid")));
        cardListContainer->addWidget(picture_widget);
    }

    QStringList unusedCards = getAllCardsForSet();
    for (const QString &cardName : possibleCards) {
        unusedCards.removeAll(cardName);
    }

    for (const QString &cardName : unusedCards) {
        CardInfoPictureWidget *picture_widget = new CardInfoPictureWidget(alreadySelectedCardListContainer);
        picture_widget->setCard(CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
            cardName,
            CardDatabaseManager::getInstance()->getSpecificSetForCard(cardName, setName, nullptr).getProperty("uuid")));
        alreadySelectedCardListContainer->addWidget(picture_widget);
    }
}
