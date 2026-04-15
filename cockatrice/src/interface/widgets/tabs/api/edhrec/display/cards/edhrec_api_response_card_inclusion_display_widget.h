/**
 * @file edhrec_api_response_card_inclusion_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 * @brief TODO: Document this.
 */

#ifndef EDHREC_API_RESPONSE_CARD_INCLUSION_DISPLAY_WIDGET_H
#define EDHREC_API_RESPONSE_CARD_INCLUSION_DISPLAY_WIDGET_H

#include "../../api_response/cards/edhrec_api_response_card_details.h"

#include <QWidget>

class QVBoxLayout;
class QLabel;
class PercentBarWidget;
class EdhrecApiResponseCardInclusionDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    EdhrecApiResponseCardInclusionDisplayWidget(QWidget *parent, const EdhrecApiResponseCardDetails &_toDisplay);
    void retranslateUi();

private:
    QVBoxLayout *layout;
    EdhrecApiResponseCardDetails toDisplay;
    QLabel *commanderLabel;
    QLabel *amountLabel;
    QLabel *inclusionLabel;
    PercentBarWidget *percentBarWidget;
};

#endif // EDHREC_API_RESPONSE_CARD_INCLUSION_DISPLAY_WIDGET_H
