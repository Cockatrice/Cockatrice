#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_LISTINGS_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_LISTINGS_DISPLAY_WIDGET_H

#include "../../../../general/layout_containers/flow_widget.h"
#include "../api_response/archidekt_deck_listing_api_response.h"

#include <QNetworkAccessManager>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class ArchidektApiResponseDeckListingsDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    void requestNavigation(QString url);

public:
    explicit ArchidektApiResponseDeckListingsDisplayWidget(QWidget *parent, ArchidektDeckListingApiResponse response);
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout *layout;
    FlowWidget *flowWidget;
    QNetworkAccessManager *imageNetworkManager;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_LISTINGS_DISPLAY_WIDGET_H
