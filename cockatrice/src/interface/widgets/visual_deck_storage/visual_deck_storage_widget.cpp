#include "visual_deck_storage_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../pixel_map_generator.h"
#include "deck_preview/deck_preview_color_identity_filter_widget.h"
#include "deck_preview/deck_preview_widget.h"
#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "visual_deck_storage_folder_display_widget.h"
#include "visual_deck_storage_model.h"
#include "visual_deck_storage_quick_settings_widget.h"
#include "visual_deck_storage_search_widget.h"
#include "visual_deck_storage_sort_filter_proxy_model.h"
#include "visual_deck_storage_sort_widget.h"
#include "visual_deck_storage_tag_filter_widget.h"

#include <QAbstractItemModel>
#include <QChar>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QScrollArea>
#include <QStringLiteral>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>
#include <algorithm>
#include <functional>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>

VisualDeckStorageWidget::VisualDeckStorageWidget(QWidget *parent) : QWidget(parent)
{
    // The model and proxy own all deck data, sorting and filtering. The view widgets below
    // only display the proxy's rows and their FilterMatchRole, so nothing touches the
    // filesystem outside the model.
    storageModel = new VisualDeckStorageModel(this);
    storageProxyModel = new VisualDeckStorageSortFilterProxyModel(this);
    storageProxyModel->setSourceModel(storageModel);

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
    refreshButton->setIcon(themePixmap(QStringLiteral("icons/reload")));
    refreshButton->setFixedSize(32, 32);
    connect(refreshButton, &QPushButton::clicked, this, &VisualDeckStorageWidget::refreshIfPossible);

    shareButton = new QToolButton(this);
    shareButton->setIcon(themePixmap(QStringLiteral("icons/share")));
    shareButton->setFixedSize(32, 32);
    shareButton->setVisible(false);
    connect(shareButton, &QPushButton::clicked, this, &VisualDeckStorageWidget::shareRequested);

    manageTagsButton = new QToolButton(this);
    connect(manageTagsButton, &QToolButton::clicked, this, &VisualDeckStorageWidget::manageTags);

    quickSettingsWidget = new VisualDeckStorageQuickSettingsWidget(this);
    connect(quickSettingsWidget, &VisualDeckStorageQuickSettingsWidget::showFoldersChanged, this,
            &VisualDeckStorageWidget::updateShowFolders);
    connect(quickSettingsWidget, &VisualDeckStorageQuickSettingsWidget::showTagFilterChanged, this,
            &VisualDeckStorageWidget::updateTagsVisibility);

    searchAndSortLayout->addWidget(deckPreviewColorIdentityFilterWidget);
    searchAndSortLayout->addWidget(sortWidget);
    searchAndSortLayout->addWidget(searchWidget);
    searchAndSortLayout->addWidget(refreshButton);
    searchAndSortLayout->addWidget(shareButton);
    searchAndSortLayout->addWidget(manageTagsButton);
    searchAndSortLayout->addWidget(quickSettingsWidget);

    // tag filter box
    tagFilterWidget = new VisualDeckStorageTagFilterWidget(this);
    tagFilterWidget->setAllTagsProvider([this] { return gatherVisibleTags(); });
    connect(tagFilterWidget, &VisualDeckStorageTagFilterWidget::filterChanged, this,
            &VisualDeckStorageWidget::updateTagFilter);
    updateTagsVisibility(SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagFilter());

    deckPreviewSelectionAnimationEnabled =
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageSelectionAnimation();
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageSelectionAnimationChanged, this,
            &VisualDeckStorageWidget::updateSelectionAnimationEnabled);

    // deck area
    scrollArea = QtUtils::makeScrollArea(this);

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
        storageProxyModel->reapplyFilters();
        // A batch of decks finished loading: re-gather the tag chips from the visible decks once
        // the burst settles instead of on every individual load.
        tagFilterWidget->refreshTags();
    });
    connect(storageModel, &QAbstractItemModel::dataChanged, this, [this] { refreshTimer->start(); });
    connect(storageModel, &VisualDeckStorageModel::deckLoaded, this, [this] { refreshTimer->start(); });
    // A deck's file path changed: re-apply the sort, since orders like "filename" depend on it.
    connect(storageModel, &VisualDeckStorageModel::deckFilePathChanged, this, [this] { storageProxyModel->resort(); });
    connect(sortWidget, &VisualDeckStorageSortWidget::sortOrderChanged, this,
            &VisualDeckStorageWidget::updateSortOrder);
    // The filter widgets only own their ui state. Pushing it into the proxy model
    // happens here, so the children stay decoupled from the model layer.
    connect(deckPreviewColorIdentityFilterWidget, &DeckPreviewColorIdentityFilterWidget::activeColorsChanged, this,
            &VisualDeckStorageWidget::updateColorFilter);
    connect(deckPreviewColorIdentityFilterWidget, &DeckPreviewColorIdentityFilterWidget::filterModeChanged, this,
            &VisualDeckStorageWidget::updateColorFilter);
    connect(searchWidget, &VisualDeckStorageSearchWidget::searchTextChanged, this,
            &VisualDeckStorageWidget::updateSearchFilter);

    // The deck content search matches card names in the configured card language;
    // re-run it whenever that setting changes so active searches follow immediately.
    CardsDisplaySettings *cardsDisplay = &SettingsCache::instance().cardsDisplay();
    const auto reapplySearchForLanguage = [this] { storageProxyModel->reapplyFilters(); };
    connect(cardsDisplay, &CardsDisplaySettings::cardLangChanged, this, reapplySearchForLanguage);
    connect(cardsDisplay, &CardsDisplaySettings::cardSearchLanguageChanged, this, reapplySearchForLanguage);

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

void VisualDeckStorageWidget::manageTags()
{
    // An incomplete scan would make a bulk edit silently miss decks.
    if (storageModel->getDeckPath().isEmpty() || storageModel->isScanning()) {
        QMessageBox::information(this, tr("Manage tags"), tr("Wait for the deck folder scan to finish first."));
        return;
    }
    for (int row = 0; row < storageModel->rowCount(); ++row) {
        const auto &deck = storageModel->dataForRow(row);
        if (!deck.loadSucceeded) {
            QMessageBox::warning(this, tr("Manage tags"),
                                 tr("Some decks are still loading or could not be read. Refresh the deck folder and "
                                    "resolve any unreadable files before changing tags across decks."));
            return;
        }
    }
    const QString folderPath = storageModel->getDeckPath();

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Manage deck tags"));
    auto *dialogLayout = new QVBoxLayout(&dialog);
    auto *explanation =
        new QLabel(tr("Rename or remove a tag from every deck in this folder and its subfolders."), &dialog);
    explanation->setWordWrap(true);
    dialogLayout->addWidget(explanation);
    auto *tags = new QListWidget(&dialog);
    dialogLayout->addWidget(tags);
    auto *buttons = new QHBoxLayout;
    auto *renameButton = new QPushButton(tr("Rename"), &dialog);
    auto *deleteButton = new QPushButton(tr("Remove"), &dialog);
    buttons->addWidget(renameButton);
    buttons->addWidget(deleteButton);
    buttons->addStretch();
    dialogLayout->addLayout(buttons);
    auto *closeButton = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(closeButton, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialogLayout->addWidget(closeButton);

    auto rebuildTags = [this, tags] {
        QStringList all = SettingsCache::instance().visualDeckStorage().getVisualDeckStorageDefaultTagsList();
        for (int row = 0; row < storageModel->rowCount(); ++row) {
            all.append(storageModel->dataForRow(row).tags);
        }
        all.removeDuplicates();
        all.removeAll(QString());
        // Default tags often begin with an emoji. Sort by the readable name,
        // not by the symbol prefix, so the catalogue is actually alphabetical.
        const auto sortKey = [](const QString &tag) {
            qsizetype first = 0;
            while (first < tag.size() && !tag.at(first).isLetterOrNumber()) {
                ++first;
            }
            return tag.mid(first).toCaseFolded();
        };
        std::sort(all.begin(), all.end(), [&sortKey](const QString &a, const QString &b) {
            const int nameOrder = QString::compare(sortKey(a), sortKey(b));
            return nameOrder == 0 ? a < b : nameOrder < 0;
        });
        tags->clear();
        tags->addItems(all);
    };
    rebuildTags();

    auto applyTag = [this, &dialog, tags, rebuildTags, folderPath](bool remove) {
        if (!tags->currentItem()) {
            return;
        }
        const QString oldTag = tags->currentItem()->text();
        QString replacement;
        if (remove) {
            if (QMessageBox::question(
                    &dialog, tr("Remove tag"),
                    tr("Remove '%1' from all decks in this folder and its subfolders?").arg(oldTag)) !=
                QMessageBox::Yes) {
                return;
            }
        } else {
            bool accepted = false;
            replacement = QInputDialog::getText(&dialog, tr("Rename tag"), tr("New tag name:"), QLineEdit::Normal,
                                                oldTag, &accepted)
                              .trimmed();
            if (!accepted || replacement == oldTag) {
                return;
            }
            if (replacement.isEmpty()) {
                QMessageBox::warning(&dialog, tr("Rename tag"), tr("A tag name cannot be empty."));
                return;
            }
        }

        if (storageModel->getDeckPath() != folderPath || storageModel->isScanning()) {
            QMessageBox::warning(&dialog, tr("Manage tags"),
                                 tr("The deck folder is being refreshed. Try again when it finishes."));
            return;
        }
        QStringList paths;
        for (int row = 0; row < storageModel->rowCount(); ++row) {
            const auto &deck = storageModel->dataForRow(row);
            if (!deck.loadSucceeded) {
                QMessageBox::warning(&dialog, tr("Manage tags"),
                                     tr("A deck is still loading or could not be read. Refresh and try again."));
                return;
            }
            if (deck.tags.contains(oldTag)) {
                paths.append(deck.filePath);
            }
        }

        QStringList failures;
        int changed = 0;
        QPointer<VisualDeckStorageModel> model(storageModel);
        for (const QString &path : paths) {
            if (!model) {
                return;
            }
            const int row = model->rowForFilePath(path);
            if (row < 0 || !model->dataForRow(row).loadSucceeded) {
                failures.append(path);
                continue;
            }
            QStringList updated = model->dataForRow(row).tags;
            for (QString &tag : updated) {
                if (tag == oldTag) {
                    tag = replacement;
                }
            }
            updated.removeAll(QString());
            updated.removeDuplicates();
            if (model->setTags(row, updated)) {
                ++changed;
            } else {
                failures.append(path);
            }
        }
        if (!model) {
            return;
        }
        if (failures.isEmpty()) {
            auto &settings = SettingsCache::instance().visualDeckStorage();
            QStringList defaults = settings.getVisualDeckStorageDefaultTagsList();
            for (QString &tag : defaults) {
                if (tag == oldTag) {
                    tag = replacement;
                }
            }
            defaults.removeAll(QString());
            defaults.removeDuplicates();
            settings.setVisualDeckStorageDefaultTagsList(defaults);
        }
        rebuildTags();
        if (failures.isEmpty()) {
            QMessageBox::information(&dialog, tr("Manage tags"), tr("Updated %n deck(s).", "", changed));
        } else {
            QMessageBox::warning(&dialog, tr("Manage tags"),
                                 tr("Updated %1 deck(s). These files were not changed; they may have been modified "
                                    "elsewhere or could not be saved:\n\n%2")
                                     .arg(changed)
                                     .arg(failures.join('\n')));
        }
    };
    connect(renameButton, &QPushButton::clicked, &dialog, [applyTag] { applyTag(false); });
    connect(deleteButton, &QPushButton::clicked, &dialog, [applyTag] { applyTag(true); });
    dialog.resize(480, 520);
    dialog.exec();
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
    if (databaseLoadIndicator) {
        databaseLoadIndicator->setText(tr("Loading database ..."));
    }

    refreshButton->setToolTip(tr("Refresh loaded files"));
    shareButton->setToolTip(tr("Select decks to share"));
    manageTagsButton->setText(tr("Manage tags"));
    manageTagsButton->setToolTip(tr("Rename or remove tags across your decks"));
    quickSettingsWidget->setToolTip(tr("Visual Deck Storage Settings"));

    sortWidget->retranslateUi();
}

void VisualDeckStorageWidget::setShareSelectable(bool selectable)
{
    if (shareSelectable == selectable) {
        return;
    }
    shareSelectable = selectable;
    if (folderWidget != nullptr) {
        folderWidget->setShareSelectable(selectable);
    }
    emit shareSelectionChanged();
}

bool VisualDeckStorageWidget::isShareSelectable() const
{
    return shareSelectable;
}

QStringList VisualDeckStorageWidget::selectedFilePaths() const
{
    QStringList selectedPaths;
    if (folderWidget != nullptr) {
        const auto previews = folderWidget->findChildren<DeckPreviewWidget *>();
        for (DeckPreviewWidget *preview : previews) {
            // Filtered-out previews stay alive hidden in their sorted place, so only decks the
            // user can actually see are part of the share.
            if (preview->isVisible() && preview->isShareSelected()) {
                selectedPaths.append(preview->filePath);
            }
        }
    }
    return selectedPaths;
}

void VisualDeckStorageWidget::clearShareSelection()
{
    if (folderWidget != nullptr) {
        const auto previews = folderWidget->findChildren<DeckPreviewWidget *>();
        for (DeckPreviewWidget *preview : previews) {
            preview->setShareSelected(false);
        }
    }
}

void VisualDeckStorageWidget::setShareAvailable(bool available)
{
    shareButton->setVisible(available);
    shareButton->setEnabled(available);
}

void VisualDeckStorageWidget::setShareSelectedFiles(const QStringList &paths)
{
    if (folderWidget != nullptr) {
        const auto previews = folderWidget->findChildren<DeckPreviewWidget *>();
        for (DeckPreviewWidget *preview : previews) {
            preview->setShareSelected(paths.contains(preview->filePath));
        }
    }
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
    storageProxyModel->setSortOrder(sortWidget->currentSortOrder());
    storageProxyModel->reapplyFilters();
}

/**
 * @brief Scans the deck folder and rebuilds the folder tree of deck previews.
 */
void VisualDeckStorageWidget::createRootFolderWidget()
{
    storageModel->setDeckPath(SettingsCache::instance().paths().getDeckPath());

    folderWidget =
        new VisualDeckStorageFolderDisplayWidget(this, this, QString(), false, quickSettingsWidget->getShowFolders());

    // QScrollArea::setWidget() deletes the widget it currently holds. If that is still the
    // database load indicator, take it back first so the label stays alive for retranslateUi().
    if (databaseLoadIndicator && scrollArea->widget() == databaseLoadIndicator) {
        scrollArea->takeWidget();
        databaseLoadIndicator->setParent(this);
        databaseLoadIndicator->setVisible(false);
    }

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
    storageProxyModel->setSortOrder(sortWidget->currentSortOrder());
}

void VisualDeckStorageWidget::updateTagFilter()
{
    const QStringList selected = tagFilterWidget->selectedTags();
    const QStringList excluded = tagFilterWidget->excludedTags();
    storageProxyModel->setTagFilter(QSet<QString>(selected.cbegin(), selected.cend()),
                                    QSet<QString>(excluded.cbegin(), excluded.cend()));
    // The visible deck set changed, so the chips are re-gathered from it.
    tagFilterWidget->refreshTags();
}

/**
 * @brief The tags of all decks currently accepted by the proxy model.
 */
QSet<QString> VisualDeckStorageWidget::gatherVisibleTags() const
{
    QSet<QString> allTags;
    for (int proxyRow = 0; proxyRow < storageProxyModel->rowCount(); ++proxyRow) {
        const QModelIndex index = storageProxyModel->index(proxyRow, 0);
        if (!index.data(VisualDeckStorageRoles::FilterMatchRole).toBool()) {
            continue;
        }
        const QStringList deckTags = index.data(VisualDeckStorageRoles::TagsRole).toStringList();
        for (const QString &tag : deckTags) {
            allTags.insert(tag);
        }
    }
    return allTags;
}

/**
 * Pushes the color identity filter widget's state into the proxy model.
 */
void VisualDeckStorageWidget::updateColorFilter()
{
    storageProxyModel->setColorFilter(deckPreviewColorIdentityFilterWidget->getFilterMode(),
                                      deckPreviewColorIdentityFilterWidget->getActiveColors());
}

/**
 * Pushes the search bar's text into the proxy model.
 */
void VisualDeckStorageWidget::updateSearchFilter(const QString &text)
{
    storageProxyModel->setSearchText(text);
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
