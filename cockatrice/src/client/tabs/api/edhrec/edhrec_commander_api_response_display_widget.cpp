#include "edhrec_commander_api_response_display_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../ui/widgets/cards/card_info_picture_widget.h"
#include "api_response/edhrec_commander_api_response.h"
#include "edhrec_commander_api_response_card_list_display_widget.h"

#include <QResizeEvent>

EdhrecCommanderApiResponseDisplayWidget::EdhrecCommanderApiResponseDisplayWidget(QWidget *parent,
                                                                                 EdhrecCommanderApiResponse response)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->setAlignment(Qt::AlignHCenter);

    auto commanderPicture = new CardInfoPictureWidget(this);
    commanderPicture->setCard(
        CardDatabaseManager::getInstance()->getCard(response.container.getCommanderDetails().getName()));
    layout->addWidget(commanderPicture);

    auto edhrec_commander_api_response_card_lists = response.container.getCardlists();
    foreach (EdhrecCommanderApiResponseCardList card_list, edhrec_commander_api_response_card_lists) {
        auto cardListDisplayWidget = new EdhrecCommanderApiResponseCardListDisplayWidget(this, card_list);
        layout->addWidget(cardListDisplayWidget);
    }
}

void EdhrecCommanderApiResponseDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qDebug() << event->size();
    layout->invalidate();
    layout->activate();
    layout->update();
}