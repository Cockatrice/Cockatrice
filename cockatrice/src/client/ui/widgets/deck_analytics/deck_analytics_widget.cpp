#include "deck_analytics_widget.h"

DeckAnalyticsWidget::DeckAnalyticsWidget(QWidget *parent, DeckListModel *_deckListModel)
    : QWidget(parent), deckListModel(_deckListModel)
{
    mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    manaCurveWidget = new ManaCurveWidget(this, deckListModel);
    mainLayout->addWidget(manaCurveWidget);

    manaDevotionWidget = new ManaDevotionWidget(this, deckListModel);
    mainLayout->addWidget(manaDevotionWidget);

    manaBaseWidget = new ManaBaseWidget(this, deckListModel);
    mainLayout->addWidget(manaBaseWidget);
}

void DeckAnalyticsWidget::refreshDisplays(DeckListModel *_deckModel)
{
    deckListModel = _deckModel;
    manaCurveWidget->setDeckModel(_deckModel);
    manaDevotionWidget->setDeckModel(_deckModel);
    manaBaseWidget->setDeckModel(_deckModel);
}
