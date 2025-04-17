#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H

#include "../../../../../ui/widgets/cards/card_info_picture_widget.h"
#include "../../api_response/commander/edhrec_commander_api_response_card_details.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class EdhrecCommanderApiResponseCardDetailsDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderApiResponseCardDetailsDisplayWidget(
        QWidget *parent,
        const EdhrecCommanderApiResponseCardDetails &_toDisplay);
public slots:
    void actRequestPageNavigation();
signals:
    void requestUrl(QString url);

private:
    EdhrecCommanderApiResponseCardDetails toDisplay;
    QVBoxLayout *layout;
    CardInfoPictureWidget *cardPictureWidget;
    QLabel *label;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
