#include "card_group_display_widget.h"

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../utility/card_info_comparator.h"
#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>

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

QWidget *CardGroupDisplayWidget::constructWidgetForIndex(int rowIndex)
{
    QPersistentModelIndex index = QPersistentModelIndex(deckListModel->index(rowIndex, 0, trackedIndex));

    if (indexToWidgetMap.contains(index)) {
        return indexToWidgetMap[index];
    }
    auto cardName = deckListModel->data(index.sibling(index.row(), 1), Qt::EditRole).toString();
    auto cardProviderId = deckListModel->data(index.sibling(index.row(), 4), Qt::EditRole).toString();

    auto widget = new CardInfoPictureWithTextOverlayWidget(getLayoutParent(), true);
    widget->setScaleFactor(cardSizeWidget->getSlider()->value());
    widget->setCard(CardDatabaseManager::getInstance()->getCard({cardName, cardProviderId}));

    connect(widget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this, &CardGroupDisplayWidget::onClick);
    connect(widget, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this, &CardGroupDisplayWidget::onHover);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, widget, &CardInfoPictureWidget::setScaleFactor);

    indexToWidgetMap.insert(index, widget);
    return widget;
}

void CardGroupDisplayWidget::updateCardDisplays()
{
    for (int i = 0; i < deckListModel->rowCount(trackedIndex); ++i) {
        addToLayout(constructWidgetForIndex(i));
    }
}

void CardGroupDisplayWidget::onCardAddition(const QModelIndex &parent, int first, int last)
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