#include "printing_selector_card_display_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../deck/deck_loader.h"
#include "../../../../deck/deck_view.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../game/cards/card_database_manager.h"

#include <QLabel>
#include <QVBoxLayout>

PrintingSelectorCardDisplayWidget::PrintingSelectorCardDisplayWidget(DeckListModel *deckModel,
                                                                     QTreeView *deckView,
                                                                     CardInfoPtr &rootCard,
                                                                     CardInfoPerSet &setInfoForCard,
                                                                     QWidget *parent)
    : QWidget(parent), deckModel(deckModel), deckView(deckView), rootCard(rootCard), setInfoForCard(setInfoForCard)
{
    layout = new QVBoxLayout();
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    cardInfoPicture = new CardInfoPictureWidget();
    cardInfoPicture->setMinimumSize(0, 0);
    qDebug() << rootCard->getName() << " " << setInfoForCard.getProperty("uuid") << " " << countCards();
    setCard = CardDatabaseManager::getInstance()->getCardByNameAndUUID(rootCard->getName(),
                                                                       setInfoForCard.getProperty("uuid"));
    cardInfoPicture->setCard(setCard);
    layout->addWidget(cardInfoPicture);

    buttonBox = new QHBoxLayout();

    incrementButton = new QPushButton("+");
    connect(incrementButton, SIGNAL(clicked()), this, SLOT(addPrinting()));
    decrementButton = new QPushButton("-");
    connect(decrementButton, SIGNAL(clicked()), this, SLOT(removePrinting()));

    cardCount = new QLabel(QString::fromStdString(std::to_string(countCards())));

    buttonBox->addWidget(decrementButton);
    buttonBox->addWidget(cardCount, 0, Qt::AlignmentFlag::AlignCenter);
    buttonBox->addWidget(incrementButton);

    QWidget *buttonBoxContainer = new QWidget();
    buttonBoxContainer->setLayout(buttonBox);
    buttonBoxContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout->addWidget(buttonBoxContainer, 0, Qt::AlignCenter);

    setName = new QLabel(setInfoForCard.getPtr()->getLongName() + " (" + setInfoForCard.getPtr()->getShortName() + ")");
    layout->addWidget(setName, 0, Qt::AlignmentFlag::AlignCenter);
    setNumber = new QLabel(setInfoForCard.getProperty("num"));
    layout->addWidget(setNumber, 0, Qt::AlignmentFlag::AlignCenter);
}

void PrintingSelectorCardDisplayWidget::addPrinting()
{
    deckModel->addCard(rootCard->getName(), setInfoForCard, DECK_ZONE_MAIN);
    cardCount->setText(QString::fromStdString(std::to_string(countCards())));
}

void PrintingSelectorCardDisplayWidget::removePrinting()
{
    decrementCardHelper(DECK_ZONE_MAIN);
    cardCount->setText(QString::fromStdString(std::to_string(countCards())));
}

void PrintingSelectorCardDisplayWidget::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    if (!idx.isValid() || offset == 0)
        return;

    const QModelIndex numberIndex = idx.sibling(idx.row(), 0);
    const int count = deckModel->data(numberIndex, Qt::EditRole).toInt();
    const int new_count = count + offset;
    deckView->setCurrentIndex(numberIndex);
    if (new_count <= 0)
        deckModel->removeRow(idx.row(), idx.parent());
    else
        deckModel->setData(numberIndex, new_count, Qt::EditRole);
}

void PrintingSelectorCardDisplayWidget::decrementCardHelper(QString zoneName)
{
    QModelIndex idx;
    idx = deckModel->findCard(setCard->getName(), zoneName, setInfoForCard.getProperty("uuid"));
    offsetCountAtIndex(idx, -1);
}

int PrintingSelectorCardDisplayWidget::countCards()
{
    int count = 0;

    if (!deckModel)
        return -1;
    DeckList *decklist = deckModel->getDeckList();
    if (!decklist)
        return -1;
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return -1;

    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            qDebug() << currentCard->getCardUuid();
            if (!currentCard)
                continue;
            for (int k = 0; k < currentCard->getNumber(); ++k) {
                if (currentCard->getCardUuid() == setInfoForCard.getProperty("uuid")) {
                    count++;
                }
            }
        }
    }
    return count;
}
