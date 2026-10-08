/**
 * @file edhrec_api_response_commander_details_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 */
//! \todo Document this file.

#ifndef EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_DISPLAY_WIDGET_H

#include "../../api_response/cards/edhrec_commander_api_response_commander_details.h"

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class EdhrecCommanderApiResponseNavigationWidget;
class CardInfoPictureWidget;
class EdhrecApiResponseCardPricesDisplayWidget;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;

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
