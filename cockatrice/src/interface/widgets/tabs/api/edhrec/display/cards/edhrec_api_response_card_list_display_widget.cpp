#include "edhrec_api_response_card_list_display_widget.h"

#include "../../../../../general/display/banner_widget.h"
#include "../../../../../general/layout_containers/flow_widget.h"
#include "../../api_response/cards/edhrec_api_response_card_details.h"
#include "../../api_response/cards/edhrec_api_response_card_list.h"
#include "edhrec_api_response_card_details_display_widget.h"

#include <QDebug>
#include <QForeach>
#include <QList>
#include <QResizeEvent>
#include <QSize>
#include <QVBoxLayout>
#include <qlogging.h>
#include <qnamespace.h>

EdhrecApiResponseCardListDisplayWidget::EdhrecApiResponseCardListDisplayWidget(QWidget *parent,
                                                                               EdhrecApiResponseCardList toDisplay)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    header = new BannerWidget(this, toDisplay.header);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    header->setBuddy(flowWidget);

    foreach (EdhrecApiResponseCardDetails card_detail, toDisplay.cardViews) {
        auto widget = new EdhrecApiResponseCardDetailsDisplayWidget(flowWidget, card_detail);
        flowWidget->addWidget(widget);
    }

    layout->addWidget(header);
    layout->addWidget(flowWidget);
}

void EdhrecApiResponseCardListDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qDebug() << event->size();
}
