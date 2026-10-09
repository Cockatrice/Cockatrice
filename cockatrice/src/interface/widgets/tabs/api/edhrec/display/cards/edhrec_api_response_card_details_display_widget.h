/**
 * @file edhrec_api_response_card_details_display_widget.h
 * @ingroup ApiResponseDisplayWidgets
 */
//! \todo Document this file.

#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H

#include "../../api_response/cards/edhrec_api_response_card_details.h"

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class BackgroundPlateWidget;
class CardInfoPictureWidget;
class EdhrecApiResponseCardInclusionDisplayWidget;
class EdhrecApiResponseCardSynergyDisplayWidget;
class QLabel;
class QVBoxLayout;

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
    BackgroundPlateWidget *backgroundPlateWidget; ///< Plate for metadata labels
    QLabel *nameLabel;
    EdhrecApiResponseCardInclusionDisplayWidget *inclusionDisplayWidget;
    EdhrecApiResponseCardSynergyDisplayWidget *synergyDisplayWidget;

protected slots:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override; ///< Hover enter
    void leaveEvent(QEvent *event) override;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_DETAILS_DISPLAY_WIDGET_H
