#include "visual_deck_storage_folder_display_widget.h"

#include "../cards/card_info_picture_widget.h"
#include "../general/display/banner_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "deck_preview/deck_preview_widget.h"
#include "visual_deck_storage_model.h"
#include "visual_deck_storage_quick_settings_widget.h"
#include "visual_deck_storage_sort_filter_proxy_model.h"
#include "visual_deck_storage_widget.h"

#include <QSet>
#include <QTimer>
#include <QVBoxLayout>

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

    auto *proxy = visualDeckStorageWidget->proxyModel();
    // A burst of proxy changes (one dataChanged per finished deck load, plus the filter
    // invalidations) coalesces into a single reconcile, so a scan of many decks doesn't
    // rebuild the flow layout once per deck.
    connect(proxy, &QAbstractItemModel::modelReset, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::rowsInserted, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::rowsRemoved, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::dataChanged, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);
    connect(proxy, &QAbstractItemModel::layoutChanged, this, &VisualDeckStorageFolderDisplayWidget::scheduleReconcile);

    reconcileTimer = new QTimer(this);
    reconcileTimer->setSingleShot(true);
    reconcileTimer->setInterval(150);
    connect(reconcileTimer, &QTimer::timeout, this, &VisualDeckStorageFolderDisplayWidget::reconcile);

    reconcile();
}

void VisualDeckStorageFolderDisplayWidget::scheduleReconcile()
{
    reconcileTimer->start();
}

/**
 * @brief Re-reads the proxy and rebuilds the deck previews and subfolder widgets to match.
 */
void VisualDeckStorageFolderDisplayWidget::reconcile()
{
    reconcileDecks();
    createSubFolderWidgets();
    refreshVisibility();
}

/**
 * @brief Reconciles the deck preview widgets in the flow layout with the accepted proxy rows.
 *
 * Decks are keyed by file path so that already-created preview widgets are reused
 * (their banner combo box, tag editor state, etc. is preserved) whenever possible.
 */
void VisualDeckStorageFolderDisplayWidget::reconcileDecks()
{
    auto *proxy = visualDeckStorageWidget->proxyModel();
    auto *model = visualDeckStorageWidget->model();

    // Collect the file paths this folder should display, in proxy (sorted) order.
    QStringList desiredOrder;
    for (int proxyRow = 0; proxyRow < proxy->rowCount(); ++proxyRow) {
        const QModelIndex sourceIndex = proxy->mapToSource(proxy->index(proxyRow, 0));
        if (!sourceIndex.isValid()) {
            continue;
        }

        const DeckPreviewData &data = model->dataForRow(sourceIndex.row());
        if (showFolders && data.folderPath != folderPath) {
            continue;
        }
        desiredOrder.append(data.filePath);
    }

    const QSet<QString> desiredSet(desiredOrder.cbegin(), desiredOrder.cend());

    // Drop previews of decks that no longer match.
    for (auto it = deckWidgets.begin(); it != deckWidgets.end();) {
        if (!desiredSet.contains(it.key())) {
            flowWidget->removeWidget(it.value());
            it.value()->deleteLater();
            it = deckWidgets.erase(it);
        } else {
            ++it;
        }
    }

    // Create or reuse the preview widgets in the desired order.
    QList<DeckPreviewWidget *> orderedWidgets;
    orderedWidgets.reserve(desiredOrder.size());
    for (const QString &filePath : desiredOrder) {
        DeckPreviewWidget *deckPreviewWidget = deckWidgets.value(filePath, nullptr);
        if (!deckPreviewWidget) {
            deckPreviewWidget = new DeckPreviewWidget(flowWidget, visualDeckStorageWidget, model, filePath);
            connect(deckPreviewWidget, &DeckPreviewWidget::deckLoadRequested, visualDeckStorageWidget,
                    &VisualDeckStorageWidget::deckLoadRequested);
            connect(deckPreviewWidget, &DeckPreviewWidget::openDeckEditor, visualDeckStorageWidget,
                    &VisualDeckStorageWidget::openDeckEditor);
            connect(visualDeckStorageWidget->settings(), &VisualDeckStorageQuickSettingsWidget::cardSizeChanged,
                    deckPreviewWidget->bannerCardDisplayWidget, &CardInfoPictureWidget::setScaleFactor);
            deckPreviewWidget->bannerCardDisplayWidget->setScaleFactor(
                visualDeckStorageWidget->settings()->getCardSize());
            deckWidgets.insert(filePath, deckPreviewWidget);
        }
        orderedWidgets.append(deckPreviewWidget);
    }

    // Re-add all widgets so the flow layout order matches the proxy order. Skipped when the
    // order is unchanged so that data-only updates don't invalidate the flow layout.
    if (desiredOrder != lastOrderedFilePaths) {
        for (DeckPreviewWidget *deckPreviewWidget : orderedWidgets) {
            flowWidget->removeWidget(deckPreviewWidget);
        }
        for (DeckPreviewWidget *deckPreviewWidget : orderedWidgets) {
            flowWidget->addWidget(deckPreviewWidget);
        }
        lastOrderedFilePaths = desiredOrder;
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

    reconcile();
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
 */
bool VisualDeckStorageFolderDisplayWidget::hasContent() const
{
    if (!deckWidgets.isEmpty()) {
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
