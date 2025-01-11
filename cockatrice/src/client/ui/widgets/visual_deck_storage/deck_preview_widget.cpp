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

    colorIdentityWidget = new DeckPreviewColorIdentityWidget(getColorIdentity());

    layout->addWidget(bannerCardDisplayWidget);
    layout->addWidget(colorIdentityWidget);
}

QString DeckPreviewWidget::getColorIdentity()
{
    QStringList cardList = deckLoader->getCardList();
    if (cardList.isEmpty()) {
        return "";
    }

    QSet<QChar> colorSet; // A set to collect unique color symbols (e.g., W, U, B, R, G)

    for (const QString &cardName : cardList) {
        CardInfoPtr currentCard = CardDatabaseManager::getInstance()->getCard(cardName);
        if (currentCard) {
            QString colors = currentCard->getColors(); // Assuming this returns something like "WUB"
            for (const QChar &color : colors) {
                colorSet.insert(color);
            }
        }
    }

    // Ensure the color identity is in WUBRG order
    QString colorIdentity;
    const QString wubrgOrder = "WUBRG";
    for (const QChar &color : wubrgOrder) {
        if (colorSet.contains(color)) {
            colorIdentity.append(color);
        }
    }

    return colorIdentity;
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