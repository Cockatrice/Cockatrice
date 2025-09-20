#include "deck_analytics_widget.h"

DeckAnalyticsWidget::DeckAnalyticsWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    container = new QWidget(scrollArea);
    containerLayout = new QVBoxLayout(container);
    container->setLayout(containerLayout);
    scrollArea->setWidget(container);

    manaCurveWidget = new ManaCurveWidget(this, deckListModel);
    containerLayout->addWidget(manaCurveWidget);

    manaDevotionWidget = new ManaDevotionWidget(this, deckListModel);
    containerLayout->addWidget(manaDevotionWidget);

    manaBaseWidget = new ManaBaseWidget(this, deckListModel);
    containerLayout->addWidget(manaBaseWidget);
}

void DeckAnalyticsWidget::refreshDisplays(DeckListModel *_deckModel)
{
    deckListModel = _deckModel;
    manaCurveWidget->setDeckModel(_deckModel);
    manaDevotionWidget->setDeckModel(_deckModel);
    manaBaseWidget->setDeckModel(_deckModel);
}
