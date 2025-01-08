#include "deck_preview_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../cards/deck_preview_card_picture_widget.h"

#include <QFileInfo>
#include <QVBoxLayout>

DeckPreviewWidget::DeckPreviewWidget(QWidget *parent, const QString &_filePath) : QWidget(parent), filePath(_filePath)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    deckLoader = new DeckLoader();
    deckLoader->loadFromFile(filePath, DeckLoader::CockatriceFormat);

    auto bannerCard = deckLoader->getBannerCard().first.isEmpty()
                          ? CardInfoPtr()
                          : CardDatabaseManager::getInstance()->getCardByNameAndProviderId(
                                deckLoader->getBannerCard().first, deckLoader->getBannerCard().second);

    bannerCardDisplayWidget = new DeckPreviewCardPictureWidget(this);

    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageClicked, this,
            &DeckPreviewWidget::imageClickedEvent);
    connect(bannerCardDisplayWidget, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
            &DeckPreviewWidget::imageDoubleClickedEvent);

    bannerCardDisplayWidget->setCard(bannerCard);
    bannerCardDisplayWidget->setOverlayText(
        deckLoader->getName().isEmpty() ? QFileInfo(deckLoader->getLastFileName()).fileName() : deckLoader->getName());
    bannerCardDisplayWidget->setFontSize(24);
    setFilePath(deckLoader->getLastFileName());

    layout->addWidget(bannerCardDisplayWidget);
}

void DeckPreviewWidget::setFilePath(const QString &_filePath)
{
    filePath = _filePath;
}

void DeckPreviewWidget::imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    Q_UNUSED(instance);
    emit deckPreviewClicked(event, this);
}

void DeckPreviewWidget::imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    Q_UNUSED(instance);
    emit deckPreviewDoubleClicked(event, this);
}