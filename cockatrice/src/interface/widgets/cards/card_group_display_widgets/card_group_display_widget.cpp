#include "card_group_display_widget.h"

#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <libcockatrice/models/deck_list/deck_list_sort_filter_proxy_model.h>

CardGroupDisplayWidget::CardGroupDisplayWidget(QWidget *parent,
                                               DeckListModel *_deckListModel,
                                               QItemSelectionModel *_selectionModel,
                                               QPersistentModelIndex _trackedIndex,
                                               QString _zoneName,
                                               QString _cardGroupCategory,
                                               QString _activeGroupCriteria,
                                               QStringList _activeSortCriteria,
                                               int bannerOpacity,
                                               CardSizeWidget *_cardSizeWidget)
    : QWidget(parent), deckListModel(_deckListModel), selectionModel(_selectionModel), trackedIndex(_trackedIndex),
      zoneName(_zoneName), cardGroupCategory(_cardGroupCategory), activeGroupCriteria(_activeGroupCriteria),
      activeSortCriteria(_activeSortCriteria), cardSizeWidget(_cardSizeWidget)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setMinimumSize(QSize(0, 0));

    banner = new BannerWidget(this, cardGroupCategory, Qt::Orientation::Vertical, bannerOpacity);

    layout->addWidget(banner);

    CardGroupDisplayWidget::updateCardDisplays();

    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &CardGroupDisplayWidget::onCardAddition);
    if (selectionModel) {
        connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
                &CardGroupDisplayWidget::onSelectionChanged);
    }
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &CardGroupDisplayWidget::onCardRemoval);
    connect(deckListModel, &QAbstractItemModel::dataChanged, this, &CardGroupDisplayWidget::onDataChanged);
}

// Just here so it can get overwritten in subclasses.
void CardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

// =====================================================================================================================
//                                                    User Interaction
// =====================================================================================================================

void CardGroupDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (selectionModel) {
        selectionModel->clearSelection();
    }
}

void CardGroupDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card);
}

void CardGroupDisplayWidget::onHover(const ExactCard &card)
{
    emit cardHovered(card);
}

void CardGroupDisplayWidget::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    auto proxyModel = qobject_cast<QAbstractProxyModel *>(selectionModel->model());

    for (auto &range : selected) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
            QModelIndex idx = range.model()->index(row, 0, range.parent());

            if (proxyModel) {
                idx = proxyModel->mapToSource(idx);
            }

            auto it = indexToWidgetMap.find(QPersistentModelIndex(idx));
            if (it != indexToWidgetMap.end()) {
                // Highlight all copies of this card
                for (auto widget : it.value()) {
                    if (auto displayWidget = qobject_cast<CardInfoPictureWithTextOverlayWidget *>(widget)) {
                        displayWidget->setHighlighted(true);
                    }
                }
            }
        }
    }

    for (auto &range : deselected) {
        for (int row = range.top(); row <= range.bottom(); ++row) {
            QModelIndex idx = range.model()->index(row, 0, range.parent());
            if (proxyModel)
                idx = proxyModel->mapToSource(idx);

            auto it = indexToWidgetMap.find(QPersistentModelIndex(idx));
            if (it != indexToWidgetMap.end()) {
                // Un-highlight all copies of this card
                for (auto widget : it.value()) {
                    if (auto displayWidget = qobject_cast<CardInfoPictureWithTextOverlayWidget *>(widget)) {
                        displayWidget->setHighlighted(false);
                    }
                }
            }
        }
    }
}

void CardGroupDisplayWidget::refreshSelectionForIndex(const QPersistentModelIndex &persistent)
{
    if (!selectionModel || !indexToWidgetMap.contains(persistent)) {
        return;
    }

    // Convert persistent index to regular index for selection check
    QModelIndex idx = QModelIndex(persistent);

    // Check if this index is selected
    // We need to check against the selection model's model (which might be a proxy)
    bool isSelected = false;
    if (auto proxyModel = qobject_cast<QAbstractProxyModel *>(selectionModel->model())) {
        // Map source index to proxy
        QModelIndex proxyIdx = proxyModel->mapFromSource(idx);
        isSelected = selectionModel->isSelected(proxyIdx);
    } else {
        isSelected = selectionModel->isSelected(idx);
    }

    // Apply selection state to all widgets for this index
    for (auto widget : indexToWidgetMap[persistent]) {
        if (auto displayWidget = qobject_cast<CardInfoPictureWithTextOverlayWidget *>(widget)) {
            displayWidget->setHighlighted(isSelected);
        }
    }
}

// =====================================================================================================================
//                                             Display Widget Management
// =====================================================================================================================

QWidget *CardGroupDisplayWidget::constructWidgetForIndex(QPersistentModelIndex index)
{
    auto cardName = index.sibling(index.row(), DeckListModelColumns::CARD_NAME).data(Qt::EditRole).toString();
    auto cardProviderId =
        index.sibling(index.row(), DeckListModelColumns::CARD_PROVIDER_ID).data(Qt::EditRole).toString();

    auto widget = new CardInfoPictureWithTextOverlayWidget(getLayoutParent(), true);
    widget->setScaleFactor(cardSizeWidget->getSlider()->value());
    widget->setCard(CardDatabaseManager::query()->getCard({cardName, cardProviderId}));

    connect(widget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this, &CardGroupDisplayWidget::onClick);
    connect(widget, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this, &CardGroupDisplayWidget::onHover);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, widget, &CardInfoPictureWidget::setScaleFactor);

    indexToWidgetMap[index].append(widget);

    return widget;
}

void CardGroupDisplayWidget::updateCardDisplays()
{
    DeckListSortFilterProxyModel proxy;
    proxy.setSourceModel(deckListModel);
    proxy.setSortCriteria(activeSortCriteria);

    // This doesn't really matter since overwrite the whole lessThan function to just compare dynamically anyway.
    proxy.setSortRole(Qt::EditRole);
    proxy.sort(DeckListModelColumns::CARD_NAME, Qt::AscendingOrder);

    // 1. trackedIndex is a source index → map it to proxy space
    QModelIndex proxyParent = proxy.mapFromSource(trackedIndex);

    // 2. iterate children under the proxy parent
    for (int i = 0; i < proxy.rowCount(proxyParent); ++i) {
        QModelIndex proxyIndex = proxy.index(i, 0, proxyParent);

        // 3. map back to source
        QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);

        // 4. persist the source index
        QPersistentModelIndex persistent(sourceIndex);

        // Get the card amount
        int cardAmount =
            sourceIndex.sibling(sourceIndex.row(), DeckListModelColumns::CARD_AMOUNT).data(Qt::EditRole).toInt();

        // Create multiple widgets for the card count
        for (int copy = 0; copy < cardAmount; ++copy) {
            addToLayout(constructWidgetForIndex(persistent));
        }
    }
}

void CardGroupDisplayWidget::clearAllDisplayWidgets()
{
    auto it = indexToWidgetMap.begin();
    while (it != indexToWidgetMap.end()) {
        for (auto displayWidget : it.value()) {
            removeFromLayout(displayWidget);
            delete displayWidget;
        }
        it = indexToWidgetMap.erase(it);
    }
}

// =====================================================================================================================
//                                           DeckListModel Signal Responses
// =====================================================================================================================

void CardGroupDisplayWidget::onCardAddition(const QModelIndex &parent, int first, int last)
{
    if (!trackedIndex.isValid()) {
        emit cleanupRequested(this);
        return;
    }

    if (parent == trackedIndex) {
        for (int row = first; row <= last; ++row) {
            QModelIndex child = deckListModel->index(row, 0, parent);

            // Persist the index
            QPersistentModelIndex persistent(child);

            // Get the card amount for the newly added card
            int cardAmount = child.sibling(child.row(), DeckListModelColumns::CARD_AMOUNT).data(Qt::EditRole).toInt();

            // Insert multiple copies
            for (int copy = 0; copy < cardAmount; ++copy) {
                insertIntoLayout(constructWidgetForIndex(persistent), row);
            }
        }
    }
}

void CardGroupDisplayWidget::onCardRemoval(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (parent != trackedIndex) {
        return;
    }

    // Use iterator so we can remove while iterating
    auto it = indexToWidgetMap.begin();
    while (it != indexToWidgetMap.end()) {
        const QPersistentModelIndex &idx = it.key();
        bool shouldRemove = !idx.isValid() || it.value().isEmpty();

        if (shouldRemove) {
            // Clean up widgets
            for (auto widget : it.value()) {
                removeFromLayout(widget);
                widget->deleteLater();
            }

            // Erase and advance iterator
            it = indexToWidgetMap.erase(it);
        } else {
            ++it;
        }
    }

    if (!trackedIndex.isValid()) {
        emit cleanupRequested(this);
    }
}

void CardGroupDisplayWidget::onDataChanged(const QModelIndex &topLeft,
                                           const QModelIndex &bottomRight,
                                           const QVector<int> &roles)
{
    if (topLeft.parent() != trackedIndex && bottomRight.parent() != trackedIndex) {
        return;
    }

    // Check if CARD_AMOUNT column changed
    bool amountChanged = (topLeft.column() <= DeckListModelColumns::CARD_AMOUNT &&
                          bottomRight.column() >= DeckListModelColumns::CARD_AMOUNT) ||
                         roles.isEmpty() || roles.contains(Qt::EditRole);

    if (!amountChanged) {
        return;
    }

    // For each affected row, adjust widget count
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        QModelIndex idx = deckListModel->index(row, 0, trackedIndex);

        if (!idx.isValid()) {
            continue;
        }

        QPersistentModelIndex persistent(idx);
        int newAmount = idx.sibling(idx.row(), DeckListModelColumns::CARD_AMOUNT).data(Qt::EditRole).toInt();

        // Get current widget count
        int currentWidgetCount = indexToWidgetMap.contains(persistent) ? indexToWidgetMap.value(persistent).count() : 0;

        if (newAmount == currentWidgetCount) {
            // Still refresh selection even if count didn't change
            refreshSelectionForIndex(persistent);
            continue;
        }

        if (newAmount < currentWidgetCount) {
            // Remove excess widgets
            int toRemove = currentWidgetCount - newAmount;

            for (int i = 0; i < toRemove; ++i) {
                if (!indexToWidgetMap[persistent].isEmpty()) {
                    QWidget *widget = indexToWidgetMap[persistent].takeLast();
                    removeFromLayout(widget);
                    widget->deleteLater();
                }
            }

            // If all widgets removed, clean up the map entry
            if (indexToWidgetMap[persistent].isEmpty()) {
                indexToWidgetMap.remove(persistent);
            }
        } else {
            // Add new widgets
            int toAdd = newAmount - currentWidgetCount;

            for (int i = 0; i < toAdd; ++i) {
                addToLayout(constructWidgetForIndex(persistent));
            }
        }

        // Always refresh selection state after modifying widgets
        refreshSelectionForIndex(persistent);
    }
}

void CardGroupDisplayWidget::onActiveSortCriteriaChanged(QStringList _activeSortCriteria)
{
    activeSortCriteria = std::move(_activeSortCriteria);

    clearAllDisplayWidgets();
    updateCardDisplays();
}