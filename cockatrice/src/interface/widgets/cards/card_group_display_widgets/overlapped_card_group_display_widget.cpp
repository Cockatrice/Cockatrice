#include "overlapped_card_group_display_widget.h"

#include "../../general/display/banner_widget.h"
#include "../../general/layout_containers/overlap_widget.h"
#include "../card_size_widget.h"

#include <QAbstractItemModel>
#include <QList>
#include <QMap>
#include <QResizeEvent>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <qnamespace.h>

class QItemSelectionModel;

OverlappedCardGroupDisplayWidget::OverlappedCardGroupDisplayWidget(QWidget *parent,
                                                                   DeckListModel *_deckListModel,
                                                                   QItemSelectionModel *_selectionModel,
                                                                   QPersistentModelIndex _trackedIndex,
                                                                   QString _zoneName,
                                                                   QString _cardGroupCategory,
                                                                   QString _activeGroupCriteria,
                                                                   QStringList _activeSortCriteria,
                                                                   int bannerOpacity,
                                                                   CardSizeWidget *_cardSizeWidget)
    : CardGroupDisplayWidget(parent,
                             _deckListModel,
                             _selectionModel,
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

    // Clear all existing widgets
    for (const QPersistentModelIndex &idx : indexToWidgetMap.keys()) {
        for (auto widget : indexToWidgetMap.value(idx)) {
            OverlappedCardGroupDisplayWidget::removeFromLayout(widget);
            widget->deleteLater();
        }
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