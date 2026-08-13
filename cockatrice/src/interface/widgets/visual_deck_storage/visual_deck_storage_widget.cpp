#include "visual_deck_storage_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../quick_settings/settings_button_widget.h"
#include "deck_preview/deck_preview_color_identity_filter_widget.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_folder_display_widget.h"
#include "visual_deck_storage_quick_settings_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_widget.h"
#include "visual_deck_storage_tag_filter_widget.h"

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent)
{
    // The model and proxy own all deck data, sorting and filtering. The view widgets below only
    // display the proxy's accepted rows, so nothing touches the filesystem outside the model.
    model_ = new VisualDeckStorageModel(this);
    proxyModel_ = new VisualDeckStorageSortFilterProxyModel(this);
    proxyModel_->setSourceModel(model_);

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
    updateTagsVisibility(SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagFilter());

    deckPreviewSelectionAnimationEnabled =
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageSelectionAnimation();
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageSelectionAnimationChanged, this,
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

    // The deck data changed (a load finished, or a mutation happened): re-evaluate the filters,
    // since search/tag/color matches are computed against the data. Debounced so that a burst of
    // load completions triggers a single re-application instead of one O(n) pass per deck.
    refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(true);
    refreshTimer->setInterval(150);
    connect(refreshTimer, &QTimer::timeout, this, [this] {
        proxyModel_->reapplyFilters();
        // A batch of decks finished loading: re-gather the tag chips from the visible decks once
        // the burst settles instead of on every individual load.
        tagFilterWidget->refreshTags();
    });
    connect(model_, &QAbstractItemModel::dataChanged, this, [this] { refreshTimer->start(); });
    connect(model_, &VisualDeckStorageModel::deckLoaded, this, [this] { refreshTimer->start(); });
    // A deck's file path changed: re-apply the sort, since orders like "filename" depend on it.
    connect(model_, &VisualDeckStorageModel::deckFilePathChanged, this, [this] { proxyModel_->resort(); });
    connect(sortWidget, &VisualDeckStorageSortWidget::sortOrderChanged, this,
            &VisualDeckStorageWidget::updateSortOrder);

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

    sortWidget->retranslateUi();
}

/**
 * Gets a const pointer to the quick settings so that the values can be accessed.
 */
const VisualDeckStorageQuickSettingsWidget *VisualDeckStorageWidget::settings() const
{
    return quickSettingsWidget;
}

/**
 * Reapplies all sort and filter options by updating the proxy model.
 */
void VisualDeckStorageWidget::reapplySortAndFilters()
{
    proxyModel_->setSortOrder(sortWidget->currentSortOrder());
    proxyModel_->reapplyFilters();
}

/**
 * @brief Scans the deck folder and rebuilds the folder tree of deck previews.
 */
void VisualDeckStorageWidget::createRootFolderWidget()
{
    model_->setDeckPath(SettingsCache::instance().paths().getDeckPath());

    folderWidget =
        new VisualDeckStorageFolderDisplayWidget(this, this, QString(), false, quickSettingsWidget->getShowFolders());

    scrollArea->setWidget(folderWidget); // this automatically destroys the old folderWidget
    scrollArea->widget()->setMaximumWidth(scrollArea->viewport()->width());
    scrollArea->widget()->adjustSize();

    // Sort and filter runs against the model data, so it is safe to apply immediately.
    reapplySortAndFilters();
}

void VisualDeckStorageWidget::updateShowFolders(bool enabled)
{
    if (folderWidget) {
        folderWidget->updateShowFolders(enabled);
    }
}

void VisualDeckStorageWidget::updateSortOrder()
{
    proxyModel_->setSortOrder(sortWidget->currentSortOrder());
}

void VisualDeckStorageWidget::updateTagFilter()
{
    const QStringList selected = tagFilterWidget->selectedTags();
    const QStringList excluded = tagFilterWidget->excludedTags();
    proxyModel_->setTagFilter(QSet<QString>(selected.cbegin(), selected.cend()),
                              QSet<QString>(excluded.cbegin(), excluded.cend()));
    // The visible deck set changed, so the chips are re-gathered from it.
    tagFilterWidget->refreshTags();
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
