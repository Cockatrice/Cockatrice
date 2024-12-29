#include "visual_deck_storage_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"
#include "visual_deck_storage_search_widget.h"

#include <QComboBox>
#include <QDirIterator>
#include <QVBoxLayout>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent), sortOrder(Alphabetical)
{
    deckListModel = new DeckListModel(this);
    deckListModel->setObjectName("visualDeckModel");

    layout = new QVBoxLayout();
    setLayout(layout);

    // ComboBox for sorting options
    sortComboBox = new QComboBox(this);
    sortComboBox->addItem("Sort Alphabetically (Filename)", Alphabetical);
    sortComboBox->addItem("Sort by Last Modified", ByLastModified);

    searchWidget = new VisualDeckStorageSearchWidget(this);

    // Add combo box to the main layout
    layout->addWidget(sortComboBox);
    layout->addWidget(searchWidget);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    cardSizeWidget = new CardSizeWidget(this, flowWidget, SettingsCache::instance().getVisualDeckStorageCardSize());
    layout->addWidget(cardSizeWidget);

    // Connect sorting change signal to refresh the file list
    connect(sortComboBox, &QComboBox::currentIndexChanged, this, [this, sortComboBox]() {
        sortOrder = static_cast<SortOrder>(sortComboBox->currentData().toInt());
        refreshBannerCards(); // Refresh the banner cards with the new sort order
    });
}

void VisualDeckStorageWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshBannerCards();
}

void VisualDeckStorageWidget::imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    emit imageClicked(event, instance);
}

void VisualDeckStorageWidget::imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance)
{
    emit imageDoubleClicked(event, instance);
}

void VisualDeckStorageWidget::refreshBannerCards()
{
    QStringList allFiles;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(SettingsCache::instance().getDeckPath(), QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    // Sort files based on the current sort order
    std::sort(allFiles.begin(), allFiles.end(), [this](const QString &file1, const QString &file2) {
        QFileInfo info1(file1);
        QFileInfo info2(file2);

        switch (sortOrder) {
            case Alphabetical:
                return info1.fileName().toLower() < info2.fileName().toLower();
            case ByLastModified:
                return info1.lastModified() > info2.lastModified();
        }

        return false; // Default case
    });

    auto filteredFiles = searchWidget->filterFiles(allFiles, searchWidget->getSearchText());

    flowWidget->clearLayout(); // Clear existing widgets in the flow layout

    foreach (const QString &file, filteredFiles) {
        auto deckLoader = new DeckLoader();
        deckLoader->loadFromFile(file, DeckLoader::CockatriceFormat);
        deckListModel->setDeckList(new DeckLoader(*deckLoader));

        auto *display = new DeckPreviewCardPictureWidget(flowWidget, false);
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
        connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, display, &CardInfoPictureWidget::setScaleFactor);
        display->setScaleFactor(cardSizeWidget->getSlider()->value());
        flowWidget->addWidget(display);
    }
}
