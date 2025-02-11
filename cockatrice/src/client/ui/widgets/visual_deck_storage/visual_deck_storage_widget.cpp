#include "visual_deck_storage_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "../../../../settings/cache_settings.h"
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
#include <qfilesystemwatcher.h>

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

    showFoldersCheckBox = new QCheckBox(this);
    showFoldersCheckBox->setChecked(SettingsCache::instance().getVisualDeckStorageShowFolders());
    connect(showFoldersCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &VisualDeckStorageWidget::updateShowFolders);
    connect(showFoldersCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckStorageShowFolders);

    tagFilterVisibilityCheckBox = new QCheckBox(this);
    tagFilterVisibilityCheckBox->setChecked(SettingsCache::instance().getVisualDeckStorageShowTagFilter());
    connect(tagFilterVisibilityCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageWidget::updateTagsVisibility);
    connect(tagFilterVisibilityCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckStorageShowTagFilter);

    tagsOnWidgetsVisibilityCheckBox = new QCheckBox(this);
    tagsOnWidgetsVisibilityCheckBox->setChecked(SettingsCache::instance().getVisualDeckStorageShowTagsOnDeckPreviews());
    connect(tagsOnWidgetsVisibilityCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckStorageShowTagsOnDeckPreviews);

    drawUnusedColorIdentitiesCheckBox = new QCheckBox(this);
    drawUnusedColorIdentitiesCheckBox->setChecked(
        SettingsCache::instance().getVisualDeckStorageDrawUnusedColorIdentities());
    connect(drawUnusedColorIdentitiesCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setVisualDeckStorageDrawUnusedColorIdentities);

    // card size slider
    cardSizeWidget = new CardSizeWidget(this, nullptr, SettingsCache::instance().getVisualDeckStorageCardSize());

    quickSettingsWidget = new SettingsButtonWidget(this);
    quickSettingsWidget->addSettingsWidget(showFoldersCheckBox);
    quickSettingsWidget->addSettingsWidget(tagFilterVisibilityCheckBox);
    quickSettingsWidget->addSettingsWidget(tagsOnWidgetsVisibilityCheckBox);
    quickSettingsWidget->addSettingsWidget(drawUnusedColorIdentitiesCheckBox);
    quickSettingsWidget->addSettingsWidget(cardSizeWidget);

    searchAndSortLayout->addWidget(deckPreviewColorIdentityFilterWidget);
    searchAndSortLayout->addWidget(sortWidget);
    searchAndSortLayout->addWidget(searchWidget);
    searchAndSortLayout->addWidget(quickSettingsWidget);

    // tag filter box
    tagFilterWidget = new VisualDeckStorageTagFilterWidget(this);
    updateTagsVisibility(SettingsCache::instance().getVisualDeckStorageShowTagFilter());

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

    addRecursiveWatch(watcher, SettingsCache::instance().getDeckPath());

    // Signals for changes
    connect(&watcher, &QFileSystemWatcher::fileChanged, [this] {
        if (scrollArea->widget() != databaseLoadIndicator) {
            createRootFolderWidget();
        }
    });

    connect(&watcher, &QFileSystemWatcher::directoryChanged, [this] {
        if (scrollArea->widget() != databaseLoadIndicator) {
            createRootFolderWidget();
        }
    });
}

void VisualDeckStorageWidget::addRecursiveWatch(QFileSystemWatcher &watcher, const QString &dirPath)
{
    QDir dir(dirPath);
    watcher.addPath(dirPath); // Watch the root directory

    for (const QFileInfo &entry : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        addRecursiveWatch(watcher, entry.absoluteFilePath());
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

    showFoldersCheckBox->setText(tr("Show Folders"));
    tagFilterVisibilityCheckBox->setText(tr("Show Tag Filter"));
    tagsOnWidgetsVisibilityCheckBox->setText(tr("Show Tags On Deck Previews"));
    drawUnusedColorIdentitiesCheckBox->setText(tr("Draw not contained Color Identities"));
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

void VisualDeckStorageWidget::createRootFolderWidget()
{
    folderWidget = new VisualDeckStorageFolderDisplayWidget(this, this, SettingsCache::instance().getDeckPath(), false,
                                                            showFoldersCheckBox->isChecked());

    scrollArea->setWidget(folderWidget);
    scrollArea->widget()->setMaximumWidth(scrollArea->viewport()->width());
    scrollArea->widget()->adjustSize();
    updateSortOrder();
}

void VisualDeckStorageWidget::updateShowFolders(bool enabled)
{
    if (folderWidget) {
        folderWidget->updateShowFolders(enabled);
        updateSortOrder();
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

void VisualDeckStorageWidget::updateTagsVisibility(const bool visible)
{
    if (visible) {
        tagFilterWidget->setVisible(true);

    } else {
        tagFilterWidget->setHidden(true);
    }
}
