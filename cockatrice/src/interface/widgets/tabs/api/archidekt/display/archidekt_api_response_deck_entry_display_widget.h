#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../api_response/deck_listings/archidekt_api_response_deck_listing_container.h"

#include <QLabel>
#include <QNetworkAccessManager>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

class ArchidektApiResponseDeckEntryDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    void requestNavigation(QString url);

public:
    explicit ArchidektApiResponseDeckEntryDisplayWidget(QWidget *parent,
                                                        ArchidektApiResponseDeckListingContainer response,
                                                        QNetworkAccessManager *imageNetworkManager);
    void onPreviewImageLoadFinished(QNetworkReply *reply);
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void actRequestNavigationToDeck();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QVBoxLayout *layout;
    ArchidektApiResponseDeckListingContainer response;
    QNetworkAccessManager *imageNetworkManager;
    QLabel *picture;
    CardInfoPictureWithTextOverlayWidget *deckPreviewDisplayWidget;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
