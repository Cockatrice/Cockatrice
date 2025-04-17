#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H

#include "../../../../../ui/widgets/cards/card_info_picture_widget.h"
#include "../../api_response/cards/edhrec_api_response_card_details.h"

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
    QLabel *label;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
