/**
 * @file edhrec_api_response_card_list_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 */
//! \todo Document this file.

#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H

#include "../../../../../general/display/banner_widget.h"

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class EdhrecApiResponseCardList;
class FlowWidget;
class QVBoxLayout;

class EdhrecApiResponseCardListDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecApiResponseCardListDisplayWidget(QWidget *parent, EdhrecApiResponseCardList toDisplay);
    void resizeEvent(QResizeEvent *event) override;
    [[nodiscard]] QString getBannerText() const
    {
        return header->getText();
    }

private:
    QVBoxLayout *layout;
    BannerWidget *header;
    FlowWidget *flowWidget;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_LIST_DISPLAY_WIDGET_H
