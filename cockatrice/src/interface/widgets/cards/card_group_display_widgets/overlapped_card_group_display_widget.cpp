#include "overlapped_card_group_display_widget.h"

#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>
#include <libcockatrice/card/card_info_comparator.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>

OverlappedCardGroupDisplayWidget::OverlappedCardGroupDisplayWidget(QWidget *parent,
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
                             _trackedIndex,
                             _zoneName,
                             _cardGroupCategory,
                             _activeGroupCriteria,
                             _activeSortCriteria,
                             bannerOpacity,
                             _cardSizeWidget)
{
    overlapWidget = new OverlapWidget(this, 80, 1, 1, Qt::Vertical, true);
    banner->setBuddy(overlapWidget);

    layout->addWidget(overlapWidget);

    for (const QPersistentModelIndex &idx : indexToWidgetMap.keys()) {
        OverlappedCardGroupDisplayWidget::removeFromLayout(indexToWidgetMap.value(idx));
        indexToWidgetMap.value(idx)->deleteLater();
        indexToWidgetMap.remove(idx);
    }

    OverlappedCardGroupDisplayWidget::updateCardDisplays();

    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, this,
            [this]() { overlapWidget->adjustMaxColumnsAndRows(); });

    disconnect(deckListModel, &QAbstractItemModel::rowsInserted, this, &CardGroupDisplayWidget::onCardAddition);
    disconnect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &CardGroupDisplayWidget::onCardRemoval);

    connect(deckListModel, &QAbstractItemModel::rowsInserted, this, &OverlappedCardGroupDisplayWidget::onCardAddition);
    connect(deckListModel, &QAbstractItemModel::rowsRemoved, this, &OverlappedCardGroupDisplayWidget::onCardRemoval);
}

void OverlappedCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    overlapWidget->resize(event->size());
    overlapWidget->adjustMaxColumnsAndRows();
}