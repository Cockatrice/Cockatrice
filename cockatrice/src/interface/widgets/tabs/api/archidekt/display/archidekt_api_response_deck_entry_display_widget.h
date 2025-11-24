#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../../../../general/layout_containers/flow_widget.h"
#include "../api_response/deck_listings/archidekt_api_response_deck_listing_container.h"

#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class ArchidektApiResponseDeckEntryDisplayWidget : public QWidget
{
    Q_OBJECT

signals:
    void requestNavigation(QString url);

public:
    explicit ArchidektApiResponseDeckEntryDisplayWidget(QWidget *parent,
                                                        ArchidektApiResponseDeckListingContainer response);
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void actRequestNavigationToDeck();

private:
    QHBoxLayout *layout;
    ArchidektApiResponseDeckListingContainer response;
    CardInfoPictureWithTextOverlayWidget *deckPreviewDisplayWidget;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
