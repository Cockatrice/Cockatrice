#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H

#include "../../../ui/widgets/general/layout_containers/flow_widget.h"
#include "api_response/edhrec_commander_api_response_card_list.h"

#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

class EdhrecCommanderApiResponseCardListDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderApiResponseCardListDisplayWidget(QWidget *parent,
                                                             EdhrecCommanderApiResponseCardList toDisplay);
    void resizeEvent(QResizeEvent *event) override;

private:
    QVBoxLayout *layout;
    FlowWidget *flowWidget;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H
