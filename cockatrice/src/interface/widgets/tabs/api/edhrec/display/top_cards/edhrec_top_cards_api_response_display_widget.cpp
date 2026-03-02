#include "edhrec_top_cards_api_response_display_widget.h"

#include "../../api_response/top_cards/edhrec_top_cards_api_response.h"
#include "../cards/edhrec_api_response_card_list_display_widget.h"

EdhrecTopCardsApiResponseDisplayWidget::EdhrecTopCardsApiResponseDisplayWidget(QWidget *parent,
                                                                               EdhrecTopCardsApiResponse response)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    cardDisplayLayout = new QVBoxLayout(this);

    // Add card list widgets
    auto edhrec_commander_api_response_card_lists = response.container.getCardlists();
    for (const EdhrecApiResponseCardList &card_list : edhrec_commander_api_response_card_lists) {
        auto cardListDisplayWidget = new EdhrecApiResponseCardListDisplayWidget(this, card_list);
        cardDisplayLayout->addWidget(cardListDisplayWidget);
    }

    // Create a QScrollArea to hold the card display widgets
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set the cardDisplayLayout inside the scroll area
    auto scrollWidget = new QWidget(scrollArea);
    scrollWidget->setLayout(cardDisplayLayout);
    scrollArea->setWidget(scrollWidget);

    layout->addWidget(scrollArea);
}

void EdhrecTopCardsApiResponseDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layout->invalidate();
    layout->activate();
    layout->update();
    if (scrollArea && scrollArea->widget()) {
        scrollArea->widget()->resize(event->size());
    }
}
