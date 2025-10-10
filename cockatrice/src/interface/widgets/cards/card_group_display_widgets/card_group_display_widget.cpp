#include "card_group_display_widget.h"

#include "../card_info_picture_with_text_overlay_widget.h"
#include "../libcockatrice_deck_list/libcockatrice/deck_list/deck_list_model.h"
#include "../libcockatrice_deck_list/libcockatrice/deck_list/deck_list_sort_filter_proxy_model.h"

#include <QResizeEvent>
#include <libcockatrice/card/card_info_comparator.h>
#include <libcockatrice/card/database/card_database_manager.h>

CardGroupDisplayWidget::CardGroupDisplayWidget(QWidget *parent,
                                               DeckListModel *_deckListModel,
                                               QPersistentModelIndex _trackedIndex,
                                               QString _zoneName,
                                               QString _cardGroupCategory,
                                               QString _activeGroupCriteria,
                                               QStringList _activeSortCriteria,
                                               int bannerOpacity,
                                               CardSizeWidget *_cardSizeWidget)
    : QWidget(parent), deckListModel(_deckListModel), trackedIndex(_trackedIndex), zoneName(_zoneName),
      cardGroupCategory(_cardGroupCategory), activeGroupCriteria(_activeGroupCriteria),
      activeSortCriteria(_activeSortCriteria), cardSizeWidget(_cardSizeWidget)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setMinimumSize(QSize(0, 0));

    banner = new BannerWidget(this, cardGroupCategory, Qt::Orientation::Vertical, bannerOpacity);

    layout->addWidget(banner);

    CardGroupDisplayWidget::updateCardDisplays();

    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &CardGroupDisplayWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &CardGroupDisplayWidget::onCardRemoval);
}

void CardGroupDisplayWidget::clearAllDisplayWidgets()
{
    for (auto idx : indexToWidgetMap.keys()) {
        auto displayWidget = indexToWidgetMap.value(idx);
        removeFromLayout(displayWidget);
        indexToWidgetMap.remove(idx);
        delete displayWidget;
    }
}

QWidget *CardGroupDisplayWidget::constructWidgetForIndex(QPersistentModelIndex index)
{
    if (indexToWidgetMap.contains(index)) {
        return indexToWidgetMap[index];
    }
    auto cardName = deckListModel->data(index.sibling(index.row(), 1), Qt::EditRole).toString();
    auto cardProviderId = deckListModel->data(index.sibling(index.row(), 4), Qt::EditRole).toString();

    auto widget = new CardInfoPictureWithTextOverlayWidget(getLayoutParent(), true);
    widget->setScaleFactor(cardSizeWidget->getSlider()->value());
    widget->setCard(CardDatabaseManager::query()->getCard({cardName, cardProviderId}));

    connect(widget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this, &CardGroupDisplayWidget::onClick);
    connect(widget, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this, &CardGroupDisplayWidget::onHover);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, widget, &CardInfoPictureWidget::setScaleFactor);

    indexToWidgetMap.insert(index, widget);
    return widget;
}

void CardGroupDisplayWidget::updateCardDisplays()
{
    DeckListSortFilterProxyModel proxy;
    proxy.setSourceModel(deckListModel);
    proxy.setSortCriteria(activeSortCriteria);

    // This doesn't really matter since overwrite the whole lessThan function to just compare dynamically anyway.
    proxy.setSortRole(Qt::EditRole);
    proxy.sort(1, Qt::AscendingOrder);

    // 1. trackedIndex is a source index → map it to proxy space
    QModelIndex proxyParent = proxy.mapFromSource(trackedIndex);

    // 2. iterate children under the proxy parent
    for (int i = 0; i < proxy.rowCount(proxyParent); ++i) {
        QModelIndex proxyIndex = proxy.index(i, 0, proxyParent);

        // 3. map back to source
        QModelIndex sourceIndex = proxy.mapToSource(proxyIndex);

        // 4. persist the source index
        QPersistentModelIndex persistent(sourceIndex);

        addToLayout(constructWidgetForIndex(persistent));
    }
}

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

            insertIntoLayout(constructWidgetForIndex(persistent), row);
        }
    }
}

void CardGroupDisplayWidget::onCardRemoval(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);
    if (parent == trackedIndex) {
        for (const QPersistentModelIndex &idx : indexToWidgetMap.keys()) {
            if (!idx.isValid()) {
                removeFromLayout(indexToWidgetMap.value(idx));
                indexToWidgetMap.value(idx)->deleteLater();
                indexToWidgetMap.remove(idx);
            }
        }

        if (!trackedIndex.isValid()) {
            emit cleanupRequested(this);
        }
    }
}

void CardGroupDisplayWidget::onActiveSortCriteriaChanged(QStringList _activeSortCriteria)
{
    activeSortCriteria = std::move(_activeSortCriteria);

    clearAllDisplayWidgets();
    updateCardDisplays();
}

void CardGroupDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card);
}

void CardGroupDisplayWidget::onHover(const ExactCard &card)
{
    emit cardHovered(card);
}

void CardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}