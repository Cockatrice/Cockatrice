#include "edhrec_commander_api_response_card_list_display_widget.h"

#include "../../../ui/widgets/general/display/banner_widget.h"
#include "edhrec_commander_api_response_card_details_display_widget.h"

#include <QLabel>

EdhrecCommanderApiResponseCardListDisplayWidget::EdhrecCommanderApiResponseCardListDisplayWidget(
    QWidget *parent,
    EdhrecCommanderApiResponseCardList toDisplay)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    auto header = new BannerWidget(toDisplay.header, Qt::Vertical, 80, this);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);

    foreach (EdhrecCommanderApiResponseCardDetails card_detail, toDisplay.cardViews) {
        auto widget = new EdhrecCommanderApiResponseCardDetailsDisplayWidget(flowWidget, card_detail);
        flowWidget->addWidget(widget);
    }

    layout->addWidget(header);
    layout->addWidget(flowWidget);
}

void EdhrecCommanderApiResponseCardListDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qDebug() << event->size();
}
