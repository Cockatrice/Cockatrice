#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H

#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../api_response/deck_listings/archidekt_api_response_deck_listing_container.h"
#include "archidekt_deck_preview_image_display_widget.h"

#include <QLabel>
#include <QNetworkAccessManager>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

class BackgroundPlateWidget;
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
    void updateScaledPreview();
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void actRequestNavigationToDeck();
    void setScaleFactor(int scale);

protected:
    void mousePressEvent(QMouseEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override; // Qt6 signature
#else
    void enterEvent(QEvent *event) override; // Qt5 signature
#endif
    void leaveEvent(QEvent *event) override;

private:
    QVBoxLayout *layout;
    ArchidektApiResponseDeckListingContainer response;
    QUrl imageUrl;
    QNetworkAccessManager *imageNetworkManager;
    ArchidektDeckPreviewImageDisplayWidget *previewWidget;
    QLabel *picture;
    QPixmap originalPixmap;
    int scaleFactor = 100;
    BackgroundPlateWidget *backgroundPlateWidget;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_ENTRY_DISPLAY_WIDGET_H
