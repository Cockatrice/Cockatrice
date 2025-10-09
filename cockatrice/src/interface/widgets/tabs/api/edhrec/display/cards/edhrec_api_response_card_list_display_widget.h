/**
 * @file edhrec_api_response_card_list_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H

#include "../../../../../general/display/banner_widget.h"
#include "../../../../../general/layout_containers/flow_widget.h"
#include "../../api_response/cards/edhrec_api_response_card_list.h"

#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

class EdhrecApiResponseCardListDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecApiResponseCardListDisplayWidget(QWidget *parent, EdhrecApiResponseCardList toDisplay);
    void resizeEvent(QResizeEvent *event) override;
    [[nodiscard]] QString getBannerText() const
    {
        return header->getText();
    };

private:
    QVBoxLayout *layout;
    BannerWidget *header;
    FlowWidget *flowWidget;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H
