#include "visual_deck_storage_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../quick_settings/settings_button_widget.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_folder_display_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_widget.h"
#include "visual_deck_storage_tag_filter_widget.h"

#include <QComboBox>
#include <QDirIterator>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent), folderWidget(nullptr)
{
    deckListModel = new DeckListModel(this);
    deckListModel->setObjectName("visualDeckModel");

    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(9, 0, 9, 5);
    setLayout(layout);

    // search bar row
    searchAndSortContainer = new QWidget(this);
    searchAndSortLayout = new QHBoxLayout(searchAndSortContainer);
    searchAndSortLayout->setSpacing(3);
    searchAndSortLayout->setContentsMargins(9, 0, 9, 0);
    searchAndSortContainer->setLayout(searchAndSortLayout);

    deckPreviewColorIdentityFilterWidget = new DeckPreviewColorIdentityFilterWidget(this);
    sortWidget = new VisualDeckStorageSortWidget(this);
    searchWidget = new VisualDeckStorageSearchWidget(this);

    refreshButton = new QToolButton(this);
    refreshButton->setIcon(QPixmap("theme:icons/reload"));
    refreshButton->setFixedSize(32, 32);
    connect(refreshButton, &QPushButton::clicked, this, &VisualDeckStorageWidget::refreshIfPossible);

    quickSettingsWidget = new VisualDeckStorageQuickSettingsWidget(this);
    connect(quickSettingsWidget, &VisualDeckStorageQuickSettingsWidget::showFoldersChanged, this,
            &VisualDeckStorageWidget::updateShowFolders);
    connect(quickSettingsWidget, &VisualDeckStorageQuickSettingsWidget::showTagFilterChanged, this,
            &VisualDeckStorageWidget::updateTagsVisibility);

    searchAndSortLayout->addWidget(deckPreviewColorIdentityFilterWidget);
    searchAndSortLayout->addWidget(sortWidget);
    searchAndSortLayout->addWidget(searchWidget);
    searchAndSortLayout->addWidget(refreshButton);
    searchAndSortLayout->addWidget(quickSettingsWidget);

    // tag filter box
    tagFilterWidget = new VisualDeckStorageTagFilterWidget(this);
    updateTagsVisibility(SettingsCache::instance().getVisualDeckStorageShowTagFilter());

    deckPreviewSelectionAnimationEnabled = SettingsCache::instance().getVisualDeckStorageSelectionAnimation();
    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageSelectionAnimationChanged, this,
            &VisualDeckStorageWidget::updateSelectionAnimationEnabled);

    // deck area
    scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);

    // putting everything together
    layout->addWidget(searchAndSortContainer);
    layout->addWidget(tagFilterWidget);
    layout->addWidget(scrollArea);

    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFinished, this,
            &VisualDeckStorageWidget::createRootFolderWidget);

    databaseLoadIndicator = new QLabel(this);
    databaseLoadIndicator->setAlignment(Qt::AlignCenter);

    retranslateUi();

    // Don't waste time processing the cards if they're going to get refreshed anyway once the db finishes loading
    if (CardDatabaseManager::getInstance()->getLoadStatus() == LoadStatus::Ok) {
        createRootFolderWidget();
        databaseLoadIndicator->setVisible(false);
    } else {
        scrollArea->setWidget(databaseLoadIndicator);
    }
}

void VisualDeckStorageWidget::refreshIfPossible()
{
    if (scrollArea->widget() != databaseLoadIndicator) {
        createRootFolderWidget();
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

    refreshButton->setToolTip(tr("Refresh loaded files"));
    quickSettingsWidget->setToolTip(tr("Visual Deck Storage Settings"));
}

/**
 * Gets a const pointer to the quick settings so that the values can be accessed.
 */
const VisualDeckStorageQuickSettingsWidget *VisualDeckStorageWidget::settings() const
{
    return quickSettingsWidget;
}

/**
 * Reapplies all sort and filter options by calling the appropriate update methods.
 */
void VisualDeckStorageWidget::reapplySortAndFilters()
{
    updateSortOrder();
    updateTagFilter();
    updateColorFilter();
    updateSearchFilter();
}

void VisualDeckStorageWidget::createRootFolderWidget()
{
    folderWidget = new VisualDeckStorageFolderDisplayWidget(this, this, SettingsCache::instance().getDeckPath(), false,
                                                            quickSettingsWidget->getShowFolders());

    scrollArea->setWidget(folderWidget); // this automatically destroys the old folderWidget
    scrollArea->widget()->setMaximumWidth(scrollArea->viewport()->width());
    scrollArea->widget()->adjustSize();

    /* We have to schedule a QTimer here so that the sorting logic doesn't try to access widgets that haven't been
     * processed by the event loop yet. Otherwise, deck sorting will intermittently segfault on some systems.
     */
    QTimer::singleShot(0, this, &VisualDeckStorageWidget::reapplySortAndFilters);
}

void VisualDeckStorageWidget::updateShowFolders(bool enabled)
{
    if (folderWidget) {
        folderWidget->updateShowFolders(enabled);
        QTimer::singleShot(0, this, &VisualDeckStorageWidget::reapplySortAndFilters);
    }
}

void VisualDeckStorageWidget::updateSortOrder()
{
    if (folderWidget) {
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
        folderWidget->updateVisibility();
    }
}

void VisualDeckStorageWidget::updateColorFilter()
{
    if (folderWidget) {
        deckPreviewColorIdentityFilterWidget->filterWidgets(folderWidget->findChildren<DeckPreviewWidget *>());
        folderWidget->updateVisibility();
    }
}

void VisualDeckStorageWidget::updateSearchFilter()
{
    if (folderWidget) {
        searchWidget->filterWidgets(folderWidget->findChildren<DeckPreviewWidget *>(), searchWidget->getSearchText());
        folderWidget->updateVisibility();
    }
}

void VisualDeckStorageWidget::updateTagsVisibility(const bool visible)
{
    if (visible) {
        tagFilterWidget->setVisible(true);

    } else {
        tagFilterWidget->setHidden(true);
    }
}

void VisualDeckStorageWidget::updateSelectionAnimationEnabled(const bool enabled)
{
    deckPreviewSelectionAnimationEnabled = enabled;
}