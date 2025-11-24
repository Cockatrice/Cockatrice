#include "archidekt_api_response_deck_entry_display_widget.h"
#include "../../../../cards/card_info_picture_with_text_overlay_widget.h"

#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHBoxLayout>
#include <QWidget>
#include <QPixmap>

ArchidektApiResponseDeckEntryDisplayWidget::ArchidektApiResponseDeckEntryDisplayWidget(
    QWidget *parent,
    ArchidektApiResponseDeckListingContainer _response,
    QNetworkAccessManager *_imageNetworkManager)
    : QWidget(parent), response(_response), imageNetworkManager(_imageNetworkManager)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    this->setMaximumWidth(200);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto headerLayout = new QVBoxLayout();

    QLabel *deckNameLabel = new QLabel(QString("%1 (%2)").arg(response.getName()).arg(response.getSize()));
    deckNameLabel->setWordWrap(true); // Allow text to wrap
    deckNameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    deckNameLabel->setMaximumWidth(200);
    headerLayout->addWidget(deckNameLabel);

    // Featured image (Deck Preview Image)
    picture = new QLabel(this);
    picture->setText("Loading Image...");
    picture->setAlignment(Qt::AlignCenter);
    picture->setMaximumWidth(200);

    QNetworkRequest req(QUrl(response.getFeatured()));
    imageNetworkManager->get(req);
    connect(imageNetworkManager, &QNetworkAccessManager::finished,
            this, &ArchidektApiResponseDeckEntryDisplayWidget::onPreviewImageLoadFinished);

    headerLayout->addWidget(picture);

    QLabel *ownerLabel = new QLabel(QString("Owner: %1").arg(response.getOwner().getName()));
    ownerLabel->setWordWrap(true);
    ownerLabel->setMaximumWidth(200);
    headerLayout->addWidget(ownerLabel);

    QLabel *edhBracketLabel = new QLabel(QString("EDH Bracket: %1").arg(response.getEDHBracket()));
    edhBracketLabel->setWordWrap(true);
    edhBracketLabel->setMaximumWidth(200);
    headerLayout->addWidget(edhBracketLabel);

    QLabel *viewCountLabel = new QLabel(QString("Views: %1").arg(response.getViewCount()));
    viewCountLabel->setWordWrap(true);
    viewCountLabel->setMaximumWidth(200);
    headerLayout->addWidget(viewCountLabel);

    QLabel *createdAtLabel = new QLabel(QString("Created: %1").arg(response.getCreatedAt()));
    createdAtLabel->setWordWrap(true);
    createdAtLabel->setMaximumWidth(200);
    QLabel *updatedAtLabel = new QLabel(QString("Updated: %1").arg(response.getUpdatedAt()));
    updatedAtLabel->setWordWrap(true);
    updatedAtLabel->setMaximumWidth(200);
    headerLayout->addWidget(createdAtLabel);
    headerLayout->addWidget(updatedAtLabel);

    QLabel *colorsLabel = new QLabel("Colors: ");
    QStringList colorNames;
    for (auto color : response.getColors().keys()) {
        colorNames << color;
    }

    if (colorNames.isEmpty()) {
        colorsLabel->setText("Colors: None");
    } else {
        colorsLabel->setText("Colors: " + colorNames.join(", "));
    }

    colorsLabel->setWordWrap(true);
    colorsLabel->setMaximumWidth(200);
    headerLayout->addWidget(colorsLabel);

    layout->addLayout(headerLayout);

    /*deckPreviewDisplayWidget = new CardInfoPictureWithTextOverlayWidget(this);
    deckPreviewDisplayWidget->setOverlayText(QString("%1 (%2)").arg(response.getName()).arg(response.getSize()));
    connect(deckPreviewDisplayWidget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this,
            &ArchidektApiResponseDeckEntryDisplayWidget::actRequestNavigationToDeck);
    layout->addWidget(deckPreviewDisplayWidget);*/
}

void ArchidektApiResponseDeckEntryDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    actRequestNavigationToDeck();
}

void ArchidektApiResponseDeckEntryDisplayWidget::onPreviewImageLoadFinished(QNetworkReply *reply)
{
    if (reply->url() != response.getFeatured()) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        picture->setText("Error loading");
        reply->deleteLater();
        return;
    }

    QByteArray imageData = reply->readAll();
    QPixmap pixmap;

    if (pixmap.loadFromData(imageData)) {
        picture->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        picture->setText("Invalid image");
    }

    reply->deleteLater();
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
