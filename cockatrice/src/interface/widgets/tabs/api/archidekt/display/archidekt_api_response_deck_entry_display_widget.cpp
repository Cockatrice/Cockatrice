#include "archidekt_api_response_deck_entry_display_widget.h"

#include "../../../../../card_picture_loader/card_picture_loader.h"
#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"
#include "../../../../general/display/background_plate_widget.h"
#include "../../../../general/display/color_bar.h"
#include "archidekt_deck_preview_image_display_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QWidget>
#include <version_string.h>

#define ARCHIDEKT_DEFAULT_IMAGE "https://storage.googleapis.com/topdekt-user/images/archidekt_deck_card_shadow.jpg"

static QString timeAgo(const QString &timestamp)
{
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);

    if (!dt.isValid())
        return timestamp; // fallback if parsing fails

    qint64 secs = dt.secsTo(QDateTime::currentDateTimeUtc());

    if (secs < 60)
        return QString("%1 seconds ago").arg(secs);
    if (secs < 3600)
        return QString("%1 minutes ago").arg(secs / 60);
    if (secs < 86400)
        return QString("%1 hours ago").arg(secs / 3600);
    if (secs < 30 * 86400)
        return QString("%1 days ago").arg(secs / 86400);
    if (secs < 365 * 86400)
        return QString("%1 months ago").arg(secs / (30 * 86400));

    return QString("%1 years ago").arg(secs / (365 * 86400));
}

ArchidektApiResponseDeckEntryDisplayWidget::ArchidektApiResponseDeckEntryDisplayWidget(
    QWidget *parent,
    ArchidektApiResponseDeckListingContainer _response,
    QNetworkAccessManager *_imageNetworkManager)
    : QWidget(parent), response(_response), imageNetworkManager(_imageNetworkManager)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    this->setMaximumWidth(400);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto headerLayout = new QVBoxLayout();

    previewWidget = new ArchidektDeckPreviewImageDisplayWidget(this);

    previewWidget->setMaximumWidth(400);
    previewWidget->setMinimumHeight(300); // consistent height

    // Set deck name (ellided)
    QFontMetrics fm(previewWidget->topLeftLabel->font());
    QString elided = fm.elidedText(response.getName(), Qt::ElideRight, 280);
    previewWidget->topLeftLabel->setLabelText(elided);
    previewWidget->topLeftLabel->setToolTip(response.getName());

    // Set count
    previewWidget->topRightLabel->setLabelText(QString::number(response.getSize()));

    // EDH bracket (skip if 0)
    if (response.getEDHBracket() != 0) {
        previewWidget->bottomLeftLabel->setLabelText(QString("EDH: %1").arg(response.getEDHBracket()));
    } else {
        previewWidget->bottomLeftLabel->hide();
    }

    // Views
    previewWidget->bottomRightLabel->setLabelText(QString("Views: %1").arg(response.getViewCount()));

    // Use preview->imageLabel for image loading
    picture = previewWidget->imageLabel;

    imageUrl = response.getFeatured().isEmpty() ? QUrl(ARCHIDEKT_DEFAULT_IMAGE) : QUrl(response.getFeatured());

    QNetworkRequest req(imageUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, QString("Cockatrice %1").arg(VERSION_STRING));
    QNetworkReply *reply = imageNetworkManager->get(req);

    // tag the reply with "this" so we know it belongs to us later
    reply->setProperty("deckWidget", QVariant::fromValue<void *>(this));
    reply->setProperty("requestedUrl", imageUrl);

    connect(imageNetworkManager, &QNetworkAccessManager::finished, this,
            &ArchidektApiResponseDeckEntryDisplayWidget::onPreviewImageLoadFinished);

    headerLayout->addWidget(previewWidget);

    auto colors = response.getColors();

    ColorBar *colorBar = new ColorBar(colors, this);
    colorBar->setMinPercentThreshold(3);
    colorBar->setFixedHeight(22);

    headerLayout->addWidget(colorBar);

    // Create a shared plate for the labels
    backgroundPlateWidget = new BackgroundPlateWidget(this);
    backgroundPlateWidget->setFixedHeight(120); // Adjust height to fit all labels

    QVBoxLayout *plateLayout = new QVBoxLayout(backgroundPlateWidget);

    // Add labels to the plate layout
    QLabel *ownerLabel = new QLabel(QString("Owner: %1").arg(response.getOwner().getName()));
    plateLayout->addWidget(ownerLabel);

    QLabel *createdAtLabel = new QLabel(QString("Created: %1").arg(timeAgo(response.getCreatedAt())));
    plateLayout->addWidget(createdAtLabel);

    QLabel *updatedAtLabel = new QLabel(QString("Updated: %1").arg(timeAgo(response.getUpdatedAt())));
    plateLayout->addWidget(updatedAtLabel);

    // Add the shared plate to the header layout
    headerLayout->addWidget(backgroundPlateWidget);

    layout->addLayout(headerLayout);
}

void ArchidektApiResponseDeckEntryDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    actRequestNavigationToDeck();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ArchidektApiResponseDeckEntryDisplayWidget::enterEvent(QEnterEvent *event)
#else
void ArchidektApiResponseDeckEntryDisplayWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    backgroundPlateWidget->setFocused(true);
}

void ArchidektApiResponseDeckEntryDisplayWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    backgroundPlateWidget->setFocused(false);
}

void ArchidektApiResponseDeckEntryDisplayWidget::setScaleFactor(int scale)
{
    scaleFactor = scale;
    updateScaledPreview();
}

void ArchidektApiResponseDeckEntryDisplayWidget::onPreviewImageLoadFinished(QNetworkReply *reply)
{
    // Check if this is our reply
    void *owner = reply->property("deckWidget").value<void *>();
    if (owner != this) {
        return; // not our reply
    }

    // Check that the requested URL matches what we asked
    QUrl requestedUrl = reply->property("requestedUrl").toUrl();
    if (requestedUrl != imageUrl) {
        reply->deleteLater();
        return;
    }

    QPixmap loaded;

    if (reply->error() != QNetworkReply::NoError || !loaded.loadFromData(reply->readAll())) {
        CardPictureLoader::getCardBackLoadingFailedPixmap(loaded, QSize(400, 400));
    }

    originalPixmap = loaded;

    // Always scale preview widget to this ratio
    previewWidget->setAspectRatio(DESIGN_RATIO);
    previewWidget->setPreviewWidth(400);

    // Initial scaling
    updateScaledPreview();

    reply->deleteLater();
}

void ArchidektApiResponseDeckEntryDisplayWidget::updateScaledPreview()
{
    if (originalPixmap.isNull()) {
        return;
    }

    int baseWidth = 400;
    int newWidth = baseWidth * scaleFactor / 100;
    int newHeight = static_cast<int>(newWidth * DESIGN_RATIO);

    previewWidget->setFixedSize(newWidth, newHeight);

    // Scale image to fill the preview area (crop edges)
    QPixmap scaled =
        originalPixmap.scaled(newWidth, newHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // Crop to exact target size
    QRect cropRect((scaled.width() - newWidth) / 2, (scaled.height() - newHeight) / 2, newWidth, newHeight);
    QPixmap cropped = scaled.copy(cropRect);

    picture->setPixmap(cropped);
    picture->setFixedSize(newWidth, newHeight);

    // Update the elided deck name based on new width
    int textMaxWidth = int(newWidth * 0.7); // allow 70% of width for text
    QFontMetrics fm(previewWidget->topLeftLabel->font());
    QString elided = fm.elidedText(response.getName(), Qt::ElideRight, textMaxWidth);
    previewWidget->topLeftLabel->setText(elided);
    previewWidget->topLeftLabel->setToolTip(response.getName());

    setFixedWidth(newWidth);

    layout->invalidate();
    layout->activate();
    updateGeometry();
}

void ArchidektApiResponseDeckEntryDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layout->invalidate();
    layout->activate();
    layout->update();
}

void ArchidektApiResponseDeckEntryDisplayWidget::actRequestNavigationToDeck()
{
    emit requestNavigation(QString("https://archidekt.com/api/decks/%1/").arg(response.getId()));
}