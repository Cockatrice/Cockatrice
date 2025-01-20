#include "edhrec_commander_api_response_card_list_display_widget.h"

#include "edhrec_commander_api_response_card_details_display_widget.h"

#include <QLabel>

EdhrecCommanderApiResponseCardListDisplayWidget::EdhrecCommanderApiResponseCardListDisplayWidget(
    QWidget *parent,
    EdhrecCommanderApiResponseCardList toDisplay)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(0, 0);

    auto header = new QLabel(this);
    header->setText(toDisplay.header);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    foreach (EdhrecCommanderApiResponseCardDetails card_detail, toDisplay.cardViews) {
        auto widget = new EdhrecCommanderApiResponseCardDetailsDisplayWidget(flowWidget, card_detail);
        flowWidget->addWidget(widget);
    }

    layout->addWidget(header);
    layout->addWidget(flowWidget);
}
