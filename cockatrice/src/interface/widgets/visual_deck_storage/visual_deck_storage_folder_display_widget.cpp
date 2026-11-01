#include "visual_deck_storage_folder_display_widget.h"

#include "../cards/card_info_picture_widget.h"
#include "../cards/card_size_widget.h"
#include "../cards/deck_preview_card_picture_widget.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_model.h"
#include "visual_deck_storage_quick_settings_widget.h"
#include "visual_deck_storage_sort_filter_proxy_model.h"
#include "visual_deck_storage_widget.h"

#include <QAbstractItemModel>
#include <QChar>
#include <QDebug>
#include <QElapsedTimer>
#include <QMetaObject>
#include <QMetaType>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>
#include <qnamespace.h>

VisualDeckStorageFolderDisplayWidget::VisualDeckStorageFolderDisplayWidget(
    QWidget *parent,
    VisualDeckStorageWidget *_visualDeckStorageWidget,
    const QString &_folderPath,
    bool canBeHidden,
    bool _showFolders)
    : QWidget(parent), showFolders(_showFolders), folderPath(_folderPath),
      visualDeckStorageWidget(_visualDeckStorageWidget)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);

    header = new BannerWidget(this, "");
    header->setClickable(canBeHidden);
    header->setHidden(!showFolders);

    const QString bannerText = folderPath.isEmpty() ? tr("Deck Storage") : folderPath;
    header->setText(bannerText);
    layout->addWidget(header);

    container = new QWidget(this);
    containerLayout = new QVBoxLayout(container);
    container->setLayout(containerLayout);

    header->setBuddy(container);

    layout->addWidget(container);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    containerLayout->addWidget(flowWidget);

    visualDeckStorageWidget->settings()->getCardSizeWidget()->enableCtrlScrollResize(this);

    auto *proxy = visualDeckStorageWidget->proxyModel();
    // A burst of proxy changes (one dataChanged per finished deck load, plus the filter
    // invalidations) coalesces into a single reconcile, so a scan of many decks doesn't
    // rebuild the flow layout once per deck.
    connect(proxy, &QAbstractItemModel::modelReset, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::rowsInserted, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::rowsRemoved, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::dataChanged, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::layoutChanged, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);

    // The preview follows its persistent model row during a rename. Keep the
    // folder's lookup in sync too, or the next pass replaces that same preview
    // and loses its share selection, focus and pending interactions.
    connect(visualDeckStorageWidget->model(), &VisualDeckStorageModel::deckFilePathChanged, this,
            [this](const QString &oldPath, const QString &newPath) {
                auto *preview = deckWidgets.take(oldPath);
                if (!preview) {
                    return;
                }
                // A removed file may have left a preview pending reconciliation
                // at the destination. The renamed row owns that path now.
                if (auto *obsolete = deckWidgets.take(newPath); obsolete && obsolete != preview) {
                    flowWidget->removeWidget(obsolete);
                    obsolete->hide();
                    obsolete->deleteLater();
                }
                deckWidgets.insert(newPath, preview);
                for (QString &path : lastOrderedFilePaths) {
                    if (path == oldPath) {
                        path = newPath;
                    }
                }
                scheduleReconcile();
            });

    reconcileTimer = new QTimer(this);
    reconcileTimer->setSingleShot(true);
    reconcileTimer->setInterval(150);
    connect(reconcileTimer, &QTimer::timeout, this, &VisualDeckStorageFolderDisplayWidget::reconcile);

    // Building the whole folder subtree synchronously here would stall the ui thread on large
    // collections, so the first reconcile runs as a chunked pass on later event loop turns.
    scheduleReconcile();
}

void VisualDeckStorageFolderDisplayWidget::scheduleReconcile()
{
    if (deckPassActive) {
        // The active pass may be scanning stale model state, so restart it from a clean
        // slate once the current chunk yields.
        deckPassRestartRequested = true;
        return;
    }
    reconcileTimer->start();
}

/**
 * @brief Starts a new chunked scan of the source model, yielding to the event loop between chunks.
 */
void VisualDeckStorageFolderDisplayWidget::reconcile()
{
    if (deckPassActive) {
        // Keep a single queued continuation even if a caller requests an
        // immediate reconcile while the previous pass is between chunks.
        deckPassRestartRequested = true;
        return;
    }
    beginDeckPass();
}

void VisualDeckStorageFolderDisplayWidget::beginDeckPass()
{
    deckPassActive = true;
    deckPassRestartRequested = false;
    deckPassRow = 0;
    visibleDeckCount = 0;
    deckPassPresentPaths.clear();
    deckPassOrderedPaths.clear();

    continueDeckPass();
}

void VisualDeckStorageFolderDisplayWidget::continueDeckPass()
{
    if (!deckPassActive) {
        return;
    }

    // Proxy rows may have moved, disappeared or reset while this continuation
    // was queued. Restart before using the old cursor against the new model.
    if (deckPassRestartRequested) {
        deckPassRestartRequested = false;
        deckPassRow = 0;
        visibleDeckCount = 0;
        deckPassPresentPaths.clear();
        deckPassOrderedPaths.clear();
    }

    QElapsedTimer passTimer;
    passTimer.start();
    const QPointer<VisualDeckStorageFolderDisplayWidget> guard(this);
    bool processedRow = false;

    auto *proxy = visualDeckStorageWidget->proxyModel();
    const int proxyRowCount = proxy->rowCount();

    // Scan rows of this folder, creating missing previews, until the time budget for this
    // event loop turn runs out. The rest continues on the next turn.
    while (deckPassRow < proxyRowCount) {
        // Skipping another folder's rows still consumes the time budget. Always
        // process one row so even a slow first lookup makes forward progress.
        if (processedRow && passTimer.elapsed() >= DECK_PASS_TIME_BUDGET_MS) {
            break;
        }
        processedRow = true;
        const int row = deckPassRow++;
        const QModelIndex index = proxy->index(row, 0);
        if (showFolders && index.data(VisualDeckStorageRoles::FolderPathRole).toString() != folderPath) {
            continue;
        }
        const QString filePath = index.data(VisualDeckStorageRoles::FilePathRole).toString();
        deckPassPresentPaths.insert(filePath);
        deckPassOrderedPaths.append(filePath);

        DeckPreviewWidget *deckPreviewWidget = deckWidgets.value(filePath, nullptr);
        if (!deckPreviewWidget) {
            deckPreviewWidget = createDeckPreviewWidget(filePath);
            flowWidget->addWidget(deckPreviewWidget);
        }

        const bool matches = index.data(VisualDeckStorageRoles::FilterMatchRole).toBool();
        deckPreviewWidget->setVisible(matches);
        if (!guard) {
            return;
        }
        if (deckPassRestartRequested) {
            break;
        }
        if (!matches) {
            // A deck that no longer matches the filters is dropped from the selection so its
            // highlight cannot linger on an invisible preview or be counted in the share.
            deckPreviewWidget->setShareSelected(false);
            // Selection observers can remove this folder or reset/filter the
            // model synchronously. Do not continue through stale rows.
            if (!guard) {
                return;
            }
            if (deckPassRestartRequested) {
                break;
            }
        }
        if (matches) {
            ++visibleDeckCount;
        }
    }

    // A restart yields just like an ordinary continuation. Recursing here can
    // monopolize the event loop (or exhaust the stack) under repeated changes.
    if (deckPassRestartRequested || deckPassRow < proxyRowCount) {
        QMetaObject::invokeMethod(this, &VisualDeckStorageFolderDisplayWidget::continueDeckPass, Qt::QueuedConnection);
        return;
    }

    finishDeckPass();
}

void VisualDeckStorageFolderDisplayWidget::finishDeckPass()
{
    // Drop previews of decks that no longer exist in the model.
    for (auto it = deckWidgets.begin(); it != deckWidgets.end();) {
        if (!deckPassPresentPaths.contains(it.key())) {
            flowWidget->removeWidget(it.value());
            it.value()->deleteLater();
            it = deckWidgets.erase(it);
        } else {
            ++it;
        }
    }

    // The chunked scan already collected proxy order. Avoid a second, unbounded
    // traversal of the entire proxy for every folder at the end of each pass.
    const QStringList &orderedFilePaths = deckPassOrderedPaths;

    // Re-add all widgets so the flow layout order matches the proxy order. Skipped when the
    // order is unchanged so that data-only updates don't invalidate the flow layout.
    if (orderedFilePaths != lastOrderedFilePaths) {
        for (const QString &filePath : orderedFilePaths) {
            flowWidget->removeWidget(deckWidgets.value(filePath));
        }
        for (const QString &filePath : orderedFilePaths) {
            flowWidget->addWidget(deckWidgets.value(filePath));
        }
        lastOrderedFilePaths = orderedFilePaths;
    }

    createSubFolderWidgets();

    // Mark completion before evaluating visibility so this pass's own numbers decide whether
    // the folder has content. The flag only guards evaluations made *during* a build.
    deckPassActive = false;
    initialPassCompleted = true;

    refreshVisibility();
}

/**
 * @brief Creates a deck preview widget and wires it up to the storage widget.
 *
 * @param filePath The absolute path of the deck file to preview.
 */
DeckPreviewWidget *VisualDeckStorageFolderDisplayWidget::createDeckPreviewWidget(const QString &filePath)
{
    auto *deckPreviewWidget =
        new DeckPreviewWidget(flowWidget, visualDeckStorageWidget, visualDeckStorageWidget->model(), filePath);
    connect(deckPreviewWidget, &DeckPreviewWidget::deckLoadRequested, visualDeckStorageWidget,
            &VisualDeckStorageWidget::deckLoadRequested);
    connect(deckPreviewWidget, &DeckPreviewWidget::openDeckEditor, visualDeckStorageWidget,
            &VisualDeckStorageWidget::openDeckEditor);
    connect(deckPreviewWidget, &DeckPreviewWidget::shareDeckRequested, visualDeckStorageWidget,
            &VisualDeckStorageWidget::shareDeckRequested);
    connect(deckPreviewWidget, &DeckPreviewWidget::shareSelectionToggled, visualDeckStorageWidget,
            &VisualDeckStorageWidget::shareSelectionChanged);
    deckPreviewWidget->setShareSelectable(visualDeckStorageWidget->isShareSelectable());
    connect(visualDeckStorageWidget->settings(), &VisualDeckStorageQuickSettingsWidget::cardSizeChanged,
            deckPreviewWidget->bannerCardDisplayWidget, &CardInfoPictureWidget::setScaleFactor);
    deckPreviewWidget->bannerCardDisplayWidget->setScaleFactor(visualDeckStorageWidget->settings()->getCardSize());
    deckWidgets.insert(filePath, deckPreviewWidget);
    return deckPreviewWidget;
}

void VisualDeckStorageFolderDisplayWidget::setShareSelectable(bool selectable)
{
    const QPointer<VisualDeckStorageFolderDisplayWidget> guard(this);
    QList<QPointer<DeckPreviewWidget>> previews;
    for (auto *preview : flowWidget->findChildren<DeckPreviewWidget *>()) {
        previews.append(preview);
    }
    for (const auto &preview : previews) {
        if (preview) {
            preview->setShareSelectable(selectable);
        }
        // Disabling sharing clears the selection and notifies external
        // observers, which may remove this folder or another preview.
        if (!guard) {
            return;
        }
    }
    QList<QPointer<VisualDeckStorageFolderDisplayWidget>> subFolders;
    for (auto *subFolder : subFolderWidgets) {
        subFolders.append(subFolder);
    }
    for (const auto &subFolder : subFolders) {
        if (subFolder) {
            subFolder->setShareSelectable(selectable);
        }
        if (!guard) {
            return;
        }
    }
}

/**
 * @brief Creates, removes and keeps in sync the subfolder widgets of this folder.
 *
 * Only direct child folders are created here; each child manages its own
 * children, mirroring the folder tree on disk.
 */
void VisualDeckStorageFolderDisplayWidget::createSubFolderWidgets()
{
    if (!showFolders) {
        return;
    }

    const QStringList children = childFolderPaths();

    for (auto it = subFolderWidgets.begin(); it != subFolderWidgets.end();) {
        if (!children.contains(it.key())) {
            containerLayout->removeWidget(it.value());
            it.value()->deleteLater();
            it = subFolderWidgets.erase(it);
        } else {
            ++it;
        }
    }

    for (const QString &child : children) {
        if (subFolderWidgets.contains(child)) {
            continue;
        }

        auto *subFolderWidget =
            new VisualDeckStorageFolderDisplayWidget(this, visualDeckStorageWidget, child, true, showFolders);
        connect(subFolderWidget, &VisualDeckStorageFolderDisplayWidget::contentVisibilityChanged, this,
                &VisualDeckStorageFolderDisplayWidget::refreshVisibility);
        containerLayout->addWidget(subFolderWidget);
        subFolderWidgets.insert(child, subFolderWidget);
    }
}

void VisualDeckStorageFolderDisplayWidget::updateShowFolders(bool enabled)
{
    showFolders = enabled;
    header->setHidden(!showFolders);

    if (!showFolders) {
        for (auto it = subFolderWidgets.begin(); it != subFolderWidgets.end(); ++it) {
            containerLayout->removeWidget(it.value());
            it.value()->deleteLater();
        }
        subFolderWidgets.clear();
    }

    scheduleReconcile();
}

/**
 * @brief Hides the folder when it contains nothing visible, and reports the change upward.
 */
void VisualDeckStorageFolderDisplayWidget::refreshVisibility()
{
    const bool shouldBeVisible = hasContent();
    if (isHidden() == !shouldBeVisible) {
        return;
    }
    setHidden(!shouldBeVisible);
    emit contentVisibilityChanged();
}

/**
 * @brief Whether this folder shows any deck previews or has any visible subfolder.
 *
 * While the first pass is still building, the folder counts as having content so it
 * doesn't flicker or hide prematurely before its previews have been created.
 */
bool VisualDeckStorageFolderDisplayWidget::hasContent() const
{
    if (!initialPassCompleted || visibleDeckCount > 0) {
        return true;
    }

    for (VisualDeckStorageFolderDisplayWidget *subFolderWidget : subFolderWidgets) {
        if (subFolderWidget->hasContent()) {
            return true;
        }
    }

    return false;
}

/**
 * @brief The direct child folder paths of this folder, sorted by name.
 */
QStringList VisualDeckStorageFolderDisplayWidget::childFolderPaths() const
{
    QStringList children;
    const QString prefix = folderPath.isEmpty() ? QString() : folderPath + "/";

    for (const QString &candidate : visualDeckStorageWidget->model()->getFolderPaths()) {
        if (!candidate.startsWith(prefix)) {
            continue;
        }
        const QString rest = candidate.mid(prefix.length());
        if (rest.isEmpty() || rest.contains('/')) {
            continue;
        }
        children.append(candidate);
    }

    return children;
}
