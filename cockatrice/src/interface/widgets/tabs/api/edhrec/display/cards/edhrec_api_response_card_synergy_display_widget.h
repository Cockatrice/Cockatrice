/**
 * @file edhrec_api_response_card_synergy_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 */
//! \todo Document this file.

#ifndef EDHREC_API_RESPONSE_CARD_SYNERGY_DISPLAY_WIDGET_H
#define EDHREC_API_RESPONSE_CARD_SYNERGY_DISPLAY_WIDGET_H

#include "../../api_response/cards/edhrec_api_response_card_details.h"

#include <QWidget>
#include <qtmetamacros.h>

class PercentBarWidget;
class QLabel;
class QVBoxLayout;

class EdhrecApiResponseCardSynergyDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    EdhrecApiResponseCardSynergyDisplayWidget(QWidget *parent, const EdhrecApiResponseCardDetails &_toDisplay);
    void retranslateUi();

private:
    QVBoxLayout *layout;
    EdhrecApiResponseCardDetails toDisplay;
    QLabel *label;
    PercentBarWidget *percentBarWidget;
};

#endif // EDHREC_API_RESPONSE_CARD_SYNERGY_DISPLAY_WIDGET_H
