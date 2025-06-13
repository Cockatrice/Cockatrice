#include "flat_card_group_display_widget.h"

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../utility/card_info_comparator.h"
#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>
#include <utility>

FlatCardGroupDisplayWidget::FlatCardGroupDisplayWidget(QWidget *parent,
                                                       DeckListModel *_deckListModel,
                                                       QPersistentModelIndex _trackedIndex,
                                                       QString _zoneName,
                                                       QString _cardGroupCategory,
                                                       QString _activeGroupCriteria,
                                                       QStringList _activeSortCriteria,
                                                       int bannerOpacity,
                                                       CardSizeWidget *_cardSizeWidget)
    : CardGroupDisplayWidget(parent,
                             _deckListModel,
                             std::move(_trackedIndex),
                             _zoneName,
                             _cardGroupCategory,
                             _activeGroupCriteria,
                             _activeSortCriteria,
                             bannerOpacity,
                             _cardSizeWidget)
{
    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    banner->setBuddy(flowWidget);

    layout->addWidget(flowWidget);

    for (const QPersistentModelIndex &idx : indexToWidgetMap.keys()) {
        FlatCardGroupDisplayWidget::removeFromLayout(indexToWidgetMap.value(idx));
        indexToWidgetMap.value(idx)->deleteLater();
        indexToWidgetMap.remove(idx);
    }

    FlatCardGroupDisplayWidget::updateCardDisplays();
    // connect(deckListModel, &DeckListModel::dataChanged, this, &FlatCardGroupDisplayWidget::updateCardDisplays);
    disconnect(deckListModel, &QAbstractItemModel::rowsInserted, this, &CardGroupDisplayWidget::onCardAddition);
    disconnect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &CardGroupDisplayWidget::onCardRemoval);

    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &FlatCardGroupDisplayWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &FlatCardGroupDisplayWidget::onCardRemoval);
}

void FlatCardGroupDisplayWidget::onCardAddition(const QModelIndex &parent, int first, int last)
{
    if (!trackedIndex.isValid()) {
        emit cleanupRequested(this);
        return;
    }
    if (parent == trackedIndex) {
        for (int i = first; i <= last; i++) {
            insertIntoLayout(constructWidgetForIndex(i), i);
        }
    }
}

void FlatCardGroupDisplayWidget::onCardRemoval(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
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

QWidget *FlatCardGroupDisplayWidget::constructWidgetForIndex(int row)
{
    QPersistentModelIndex index = QPersistentModelIndex(deckListModel->index(row, 0, trackedIndex));

    if (indexToWidgetMap.contains(index)) {
        return indexToWidgetMap[index];
    }
    auto cardName = deckListModel->data(index.sibling(index.row(), 1), Qt::EditRole).toString();
    auto cardProviderId = deckListModel->data(index.sibling(index.row(), 4), Qt::EditRole).toString();

    auto widget = new CardInfoPictureWithTextOverlayWidget(flowWidget, true);
    widget->setScaleFactor(cardSizeWidget->getSlider()->value());
    widget->setCard(CardDatabaseManager::getInstance()->getCardByNameAndProviderId(cardName, cardProviderId));

    connect(widget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this, &FlatCardGroupDisplayWidget::onClick);
    connect(widget, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this, &FlatCardGroupDisplayWidget::onHover);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, widget, &CardInfoPictureWidget::setScaleFactor);

    indexToWidgetMap.insert(index, widget);
    return widget;
}

void FlatCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}