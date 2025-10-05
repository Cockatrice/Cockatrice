#include "flat_card_group_display_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>
#include <libcockatrice/card/card_database/card_database_manager.h>
#include <libcockatrice/card/card_info_comparator.h>
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
    disconnect(deckListModel, &QAbstractItemModel::rowsInserted, this, &CardGroupDisplayWidget::onCardAddition);
    disconnect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &CardGroupDisplayWidget::onCardRemoval);

    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &FlatCardGroupDisplayWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &FlatCardGroupDisplayWidget::onCardRemoval);
}

void FlatCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}