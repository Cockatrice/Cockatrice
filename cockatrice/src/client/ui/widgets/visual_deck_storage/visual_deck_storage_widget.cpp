#include "visual_deck_storage_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"

#include <QDirIterator>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent)
{
    deckListModel = new DeckListModel(this);
    deckListModel->setObjectName("visualDeckModel");

    layout = new QHBoxLayout(this);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    getBannerCardsForDecks();
}

void VisualDeckStorageWidget::imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    emit imageClicked(event, instance);
}

void VisualDeckStorageWidget::imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    emit imageDoubleClicked(event, instance);
}

QStringList VisualDeckStorageWidget::getBannerCardsForDecks()
{
    QStringList allFiles;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(SettingsCache::instance().getDeckPath(), QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    foreach (const QString &file, allFiles) {
        qDebug() << file;
        auto deckLoader = new DeckLoader();
        deckLoader->loadFromFile(file, DeckLoader::CockatriceFormat);
        deckListModel->setDeckList(new DeckLoader(*deckLoader));

        auto *display = new DeckPreviewCardPictureWidget(flowWidget, true);
        qDebug() << "Banner card is: " << deckLoader->getBannerCard();
        auto bannerCard = deckLoader->getBannerCard().isEmpty()
                              ? CardInfoPtr()
                              : CardDatabaseManager::getInstance()->getCard(deckLoader->getBannerCard());
        display->setCard(bannerCard);
        display->setOverlayText(deckLoader->getName().isEmpty() ? QFileInfo(deckLoader->getLastFileName()).fileName()
                                                                : deckLoader->getName());
        display->setFontSize(24);
        display->setFilePath(deckLoader->getLastFileName());

        connect(display, &DeckPreviewCardPictureWidget::imageClicked, this,
                &VisualDeckStorageWidget::imageClickedEvent);
        connect(display, &DeckPreviewCardPictureWidget::imageDoubleClicked, this,
                &VisualDeckStorageWidget::imageDoubleClickedEvent);
        flowWidget->addWidget(display);
    }

    return QStringList("lol");
}