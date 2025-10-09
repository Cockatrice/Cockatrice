/**
 * @file edhrec_api_response_commander_details_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H

#include "../../../../../cards/card_info_picture_widget.h"
#include "../../api_response/cards/edhrec_commander_api_response_commander_details.h"
#include "../card_prices/edhrec_api_response_card_prices_display_widget.h"
#include "edhrec_commander_api_response_navigation_widget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class EdhrecCommanderApiResponseNavigationWidget;
class EdhrecCommanderResponseCommanderDetailsDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecCommanderResponseCommanderDetailsDisplayWidget(
        QWidget *parent,
        const EdhrecCommanderApiResponseCommanderDetails &_commanderDetails,
        QString baseUrl);
    void retranslateUi();

private:
    EdhrecCommanderApiResponseCommanderDetails commanderDetails;
    QVBoxLayout *layout;
    CardInfoPictureWidget *commanderPicture;
    QLabel *label;
    QLabel *salt;
    EdhrecApiResponseCardPricesDisplayWidget *cardPricesDisplayWidget;
    EdhrecCommanderApiResponseNavigationWidget *navigationWidget;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H
