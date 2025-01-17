#include "visual_deck_storage_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_widget.h"
#include "visual_deck_storage_tag_filter_widget.h"

#include <QComboBox>
#include <QDirIterator>
#include <QMouseEvent>
#include <QVBoxLayout>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent)
{
    deckListModel = new DeckListModel(this);
    deckListModel->setObjectName("visualDeckModel");

    layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(9, 0, 9, 5);
    setLayout(layout);

    searchAndSortLayout = new QHBoxLayout();
    searchAndSortLayout->setSpacing(3);
    searchAndSortLayout->setContentsMargins(9, 0, 9, 0);

    deckPreviewColorIdentityFilterWidget = new DeckPreviewColorIdentityFilterWidget(this);
    sortWidget = new VisualDeckStorageSortWidget(this);
    searchWidget = new VisualDeckStorageSearchWidget(this);
    tagFilterWidget = new VisualDeckStorageTagFilterWidget(this);

    searchAndSortLayout->addWidget(deckPreviewColorIdentityFilterWidget);
    searchAndSortLayout->addWidget(sortWidget);
    searchAndSortLayout->addWidget(searchWidget);
    layout->addLayout(searchAndSortLayout);
    layout->addWidget(tagFilterWidget);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    cardSizeWidget = new CardSizeWidget(this, flowWidget, SettingsCache::instance().getVisualDeckStorageCardSize());
    layout->addWidget(cardSizeWidget);

    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
            &VisualDeckStorageWidget::refreshBannerCards);

    databaseLoadIndicator = new QLabel(this);
    databaseLoadIndicator->setAlignment(Qt::AlignCenter);

    retranslateUi();

    // Don't waste time processing the cards if they're going to get refreshed anyway once the db finishes loading
    if (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok) {
        refreshBannerCards();
        databaseLoadIndicator->setVisible(false);
    } else {
        flowWidget->addWidget(databaseLoadIndicator);
    }
}

void VisualDeckStorageWidget::retranslateUi()
{
    databaseLoadIndicator->setText(tr("Loading database ..."));
}

void VisualDeckStorageWidget::updateSortOrder()
{
    refreshBannerCards(); // Refresh the banner cards with the new sort order
}

void VisualDeckStorageWidget::deckPreviewClickedEvent(QMouseEvent *event, DeckPreviewWidget *instance)
{
    emit deckPreviewClicked(event, instance);
}

void VisualDeckStorageWidget::deckPreviewDoubleClickedEvent(QMouseEvent *event, DeckPreviewWidget *instance)
{
    emit deckPreviewDoubleClicked(event, instance);
}

void VisualDeckStorageWidget::refreshBannerCards()
{
    QStringList allFiles;
    QList<DeckPreviewWidget *> allDecks;

    // QDirIterator with QDir::Files and QDir::NoSymLinks ensures only files are listed (no directories or symlinks)
    QDirIterator it(SettingsCache::instance().getDeckPath(), QDir::Files | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        allFiles << it.next(); // Add each file path to the list
    }

    for (const QString &file : allFiles) {
        auto *display = new DeckPreviewWidget(this, file);

        connect(display, &DeckPreviewWidget::deckPreviewClicked, this,
                &VisualDeckStorageWidget::deckPreviewClickedEvent);
        connect(display, &DeckPreviewWidget::deckPreviewDoubleClicked, this,
                &VisualDeckStorageWidget::deckPreviewDoubleClickedEvent);
        connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, display->bannerCardDisplayWidget,
                &CardInfoPictureWidget::setScaleFactor);
        display->bannerCardDisplayWidget->setScaleFactor(cardSizeWidget->getSlider()->value());
        allDecks.append(display);
    }

    auto filteredByColorIdentity =
        deckPreviewColorIdentityFilterWidget->filterWidgets(sortWidget->filterFiles(allDecks));
    auto filteredByTags = tagFilterWidget->filterDecksBySelectedTags(filteredByColorIdentity);
    auto filteredFiles = searchWidget->filterFiles(filteredByTags, searchWidget->getSearchText());

    tagFilterWidget->removeTagsNotInList(gatherAllTags(filteredFiles));
    tagFilterWidget->addTagsIfNotPresent(gatherAllTags(filteredFiles));

    flowWidget->clearLayout(); // Clear existing widgets in the flow layout

    for (DeckPreviewWidget *deck : filteredFiles) {
        flowWidget->addWidget(deck);
    }

    emit bannerCardsRefreshed();
}

QStringList VisualDeckStorageWidget::gatherAllTagsFromFlowWidget() const
{
    QStringList allTags;

    if (flowWidget) {
        // Iterate through all DeckPreviewWidgets
        for (DeckPreviewWidget *display : flowWidget->findChildren<DeckPreviewWidget *>()) {
            // Get tags from each DeckPreviewWidget
            QStringList tags = display->deckLoader->getTags();

            // Add tags to the list while avoiding duplicates
            allTags.append(tags);
        }
    }

    // Remove duplicates by calling 'removeDuplicates'
    allTags.removeDuplicates();

    return allTags;
}

QStringList VisualDeckStorageWidget::gatherAllTags(const QList<DeckPreviewWidget *> &allDecks)
{
    QStringList allTags;

    // Iterate through all decks provided as input
    for (DeckPreviewWidget *deck : allDecks) {
        QStringList tags = deck->deckLoader->getTags();

        // Add tags to the list while avoiding duplicates
        allTags.append(tags);
    }

    // Remove duplicates
    allTags.removeDuplicates();

    return allTags;
}
