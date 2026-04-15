/**
 * @file edhrec_api_response_commander_details_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H

#include "../../api_response/cards/edhrec_commander_api_response_commander_details.h"

#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class CardInfoPictureWidget;
class QLabel;
class EdhrecApiResponseCardPricesDisplayWidget;
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
    QHBoxLayout *layout;
    QHBoxLayout *commanderLayout;
    QVBoxLayout *commanderDetailsLayout;
    QVBoxLayout *navigationAndPricesLayout;
    CardInfoPictureWidget *commanderPicture;
    QLabel *commanderName;
    QLabel *label;
    QLabel *salt;
    EdhrecApiResponseCardPricesDisplayWidget *cardPricesDisplayWidget;
    EdhrecCommanderApiResponseNavigationWidget *navigationWidget;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H
