/**
 * @file edhrec_api_response_card_details_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H

#include "../../../../../cards/card_info_picture_widget.h"
#include "../../api_response/cards/edhrec_api_response_card_details.h"
#include "edhrec_api_response_card_inclusion_display_widget.h"
#include "edhrec_api_response_card_synergy_display_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class EdhrecApiResponseCardDetailsDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EdhrecApiResponseCardDetailsDisplayWidget(QWidget *parent, const EdhrecApiResponseCardDetails &_toDisplay);
public slots:
    void actRequestPageNavigation();
signals:
    void requestUrl(QString url);

private:
    EdhrecApiResponseCardDetails toDisplay;
    QVBoxLayout *layout;
    CardInfoPictureWidget *cardPictureWidget;
    QLabel *nameLabel;
    EdhrecApiResponseCardInclusionDisplayWidget *inclusionDisplayWidget;
    EdhrecApiResponseCardSynergyDisplayWidget *synergyDisplayWidget;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
