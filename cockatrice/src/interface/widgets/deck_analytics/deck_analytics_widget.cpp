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

    deckListStatisticsAnalyzer = new DeckListStatisticsAnalyzer(this, deckListModel);

    manaCurveWidget = new ManaCurveWidget(this, deckListStatisticsAnalyzer);
    containerLayout->addWidget(manaCurveWidget);

    manaDevotionWidget = new ManaDevotionWidget(this, deckListStatisticsAnalyzer);
    containerLayout->addWidget(manaDevotionWidget);

    manaBaseWidget = new ManaBaseWidget(this, deckListStatisticsAnalyzer);
    containerLayout->addWidget(manaBaseWidget);
}

void DeckAnalyticsWidget::refreshDisplays()
{
    deckListStatisticsAnalyzer->update();
}
