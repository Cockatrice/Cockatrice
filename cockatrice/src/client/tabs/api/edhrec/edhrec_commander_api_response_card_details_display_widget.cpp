#include "edhrec_commander_api_response_card_details_display_widget.h"

#include "../../../../game/cards/card_database_manager.h"

EdhrecCommanderApiResponseCardDetailsDisplayWidget::EdhrecCommanderApiResponseCardDetailsDisplayWidget(
    QWidget *parent,
    EdhrecCommanderApiResponseCardDetails toDisplay)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);
    cardPictureWidget = new CardInfoPictureWidget(this);
    cardPictureWidget->setCard(CardDatabaseManager::getInstance()->getCard(toDisplay.name));
    layout->addWidget(cardPictureWidget);
}
