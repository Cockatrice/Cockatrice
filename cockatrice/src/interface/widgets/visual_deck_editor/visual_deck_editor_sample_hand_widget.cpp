#include "visual_deck_editor_sample_hand_widget.h"

#include "../../../deck/deck_loader.h"
#include "../cards/card_info_picture_widget.h"
#include "card/card_database/card_database_manager.h"
#include "settings/cache_settings.h"

#include <random>

VisualDeckEditorSampleHandWidget::VisualDeckEditorSampleHandWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    resetAndHandSizeContainerWidget = new QWidget(this);
    resetAndHandSizeLayout = new QHBoxLayout(resetAndHandSizeContainerWidget);
    resetAndHandSizeContainerWidget->setLayout(resetAndHandSizeLayout);

    resetButton = new QPushButton(this);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(updateDisplay()));
    resetAndHandSizeLayout->addWidget(resetButton);

    handSizeSpinBox = new QSpinBox(this);
    handSizeSpinBox->setValue(SettingsCache::instance().getVisualDeckEditorSampleHandSize());
    handSizeSpinBox->setMinimum(1);
    connect(handSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), &SettingsCache::instance(),
            &SettingsCache::setVisualDeckEditorSampleHandSize);
    connect(handSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &VisualDeckEditorSampleHandWidget::updateDisplay);
    resetAndHandSizeLayout->addWidget(handSizeSpinBox);

    layout->addWidget(resetAndHandSizeContainerWidget);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    cardSizeWidget = new CardSizeWidget(this, flowWidget);
    layout->addWidget(cardSizeWidget);

    for (const ExactCard &card : getRandomCards(handSizeSpinBox->value())) {
        auto displayWidget = new CardInfoPictureWidget(this);
        displayWidget->setCard(card);
        displayWidget->setScaleFactor(cardSizeWidget->getSlider()->value());
        flowWidget->addWidget(displayWidget);
    }

    retranslateUi();
}

void VisualDeckEditorSampleHandWidget::retranslateUi()
{
    resetButton->setText(tr("Draw a new sample hand"));
    handSizeSpinBox->setToolTip(tr("Sample hand size"));
}

void VisualDeckEditorSampleHandWidget::setDeckModel(DeckListModel *deckModel)
{
    deckListModel = deckModel;
    // connect(deckListModel, &DeckListModel::dataChanged, this, &VisualDeckEditorSampleHandWidget::updateDisplay);
    updateDisplay();
}

void VisualDeckEditorSampleHandWidget::updateDisplay()
{
    flowWidget->clearLayout();
    for (const ExactCard &card : getRandomCards(handSizeSpinBox->value())) {
        auto displayWidget = new CardInfoPictureWidget(this);
        displayWidget->setCard(card);
        displayWidget->setScaleFactor(cardSizeWidget->getSlider()->value());
        connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, displayWidget,
                &CardInfoPictureWidget::setScaleFactor);
        flowWidget->addWidget(displayWidget);
    }
}

QList<ExactCard> VisualDeckEditorSampleHandWidget::getRandomCards(int amountToGet)
{
    QList<ExactCard> mainDeckCards;
    QList<ExactCard> randomCards;
    if (!deckListModel)
        return randomCards;
    DeckList *decklist = deckListModel->getDeckList();
    if (!decklist)
        return randomCards;
    InnerDecklistNode *listRoot = decklist->getRoot();
    if (!listRoot)
        return randomCards;

    // Collect all cards in the main deck, allowing duplicates based on their count
    for (int i = 0; i < listRoot->size(); i++) {
        InnerDecklistNode *currentZone = dynamic_cast<InnerDecklistNode *>(listRoot->at(i));
        if (!currentZone)
            continue;
        if (currentZone->getName() != DECK_ZONE_MAIN)
            continue; // Only process the main deck

        for (int j = 0; j < currentZone->size(); j++) {
            DecklistCardNode *currentCard = dynamic_cast<DecklistCardNode *>(currentZone->at(j));
            if (!currentCard)
                continue;

            for (int k = 0; k < currentCard->getNumber(); ++k) {
                ExactCard card = CardDatabaseManager::query()->getCard(currentCard->toCardRef());
                if (card) {
                    mainDeckCards.append(card);
                }
            }
        }
    }

    if (mainDeckCards.isEmpty())
        return randomCards;

    // Shuffle the deck
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(mainDeckCards.begin(), mainDeckCards.end(), rng);

    // Select amountToGet cards

    for (int i = 0; i < qMin(amountToGet, mainDeckCards.size()); ++i) {
        randomCards.append(mainDeckCards.at(i));
    }

    std::sort(randomCards.begin(), randomCards.end(),
              [](const ExactCard &a, const ExactCard &b) { return a.getInfo().getCmc() < b.getInfo().getCmc(); });

    return randomCards;
}
