#include "visual_deck_storage_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_folder_display_widget.h"
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

    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(9, 0, 9, 5);
    setLayout(layout);

    searchAndSortLayout = new QHBoxLayout(this);
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
    cardSizeWidget = new CardSizeWidget(this, nullptr, SettingsCache::instance().getVisualDeckStorageCardSize());

    scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    folderWidget = new VisualDeckStorageFolderDisplayWidget(this, this, SettingsCache::instance().getDeckPath(), false);

    scrollArea->setWidget(folderWidget);
    scrollArea->setWidgetResizable(true);

    layout->addWidget(scrollArea);

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
        scrollArea->setWidget(databaseLoadIndicator);
    }
}

void VisualDeckStorageWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (scrollArea->widget() == folderWidget) {
        scrollArea->widget()->setMaximumWidth(scrollArea->viewport()->width());
        scrollArea->widget()->adjustSize();
    }
}

void VisualDeckStorageWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (scrollArea->widget() == folderWidget) {
        scrollArea->widget()->setMaximumWidth(scrollArea->viewport()->width());
        scrollArea->widget()->adjustSize();
    }
}

void VisualDeckStorageWidget::retranslateUi()
{
    databaseLoadIndicator->setText(tr("Loading database ..."));
}

void VisualDeckStorageWidget::deckPreviewClickedEvent(QMouseEvent *event, DeckPreviewWidget *instance)
{
    emit deckPreviewClicked(event, instance);
}

void VisualDeckStorageWidget::deckPreviewDoubleClickedEvent(QMouseEvent *event, DeckPreviewWidget *instance)
{
    emit deckPreviewDoubleClicked(event, instance);
    emit deckLoadRequested(instance->filePath);
}

void VisualDeckStorageWidget::refreshBannerCards()
{
    folderWidget = new VisualDeckStorageFolderDisplayWidget(this, this, SettingsCache::instance().getDeckPath(), false);
    scrollArea->setWidget(folderWidget);
    scrollArea->widget()->setMaximumWidth(scrollArea->viewport()->width());
    scrollArea->widget()->adjustSize();
}

void VisualDeckStorageWidget::updateSortOrder()
{
    if (folderWidget) {
        qDebug() << "Updating sort order";
        sortWidget->sortFolder(folderWidget);
        for (VisualDeckStorageFolderDisplayWidget *subFolderWidget :
             folderWidget->findChildren<VisualDeckStorageFolderDisplayWidget *>()) {
            sortWidget->sortFolder(subFolderWidget);
        }
    }
}

void VisualDeckStorageWidget::updateTagFilter()
{
    if (folderWidget) {
        tagFilterWidget->filterDecksBySelectedTags(folderWidget->findChildren<DeckPreviewWidget *>());
    }
    emit tagFilterUpdated();
}

void VisualDeckStorageWidget::updateColorFilter()
{
    if (folderWidget) {
        deckPreviewColorIdentityFilterWidget->filterWidgets(folderWidget->findChildren<DeckPreviewWidget *>());
    }
    emit colorFilterUpdated();
}

void VisualDeckStorageWidget::updateSearchFilter()
{
    if (folderWidget) {
        searchWidget->filterWidgets(folderWidget->findChildren<DeckPreviewWidget *>(), searchWidget->getSearchText());
    }
    emit searchFilterUpdated();
}
