#include "card_group_display_widget.h"

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../utility/card_info_comparator.h"
#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>

CardGroupDisplayWidget::CardGroupDisplayWidget(QWidget *parent,
                                               DeckListModel *_deckListModel,
                                               QString _zoneName,
                                               QString _cardGroupCategory,
                                               QString _activeGroupCriteria,
                                               QStringList _activeSortCriteria,
                                               int bannerOpacity,
                                               CardSizeWidget *_cardSizeWidget)
    : QWidget(parent), deckListModel(_deckListModel), zoneName(_zoneName), cardGroupCategory(_cardGroupCategory),
      activeGroupCriteria(_activeGroupCriteria), activeSortCriteria(_activeSortCriteria),
      cardSizeWidget(_cardSizeWidget)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setMinimumSize(QSize(0, 0));

    banner = new BannerWidget(this, cardGroupCategory, Qt::Orientation::Vertical, bannerOpacity);

    layout->addWidget(banner);
    CardGroupDisplayWidget::updateCardDisplays();
}

void CardGroupDisplayWidget::updateCardDisplays()
{
}

QList<CardInfoPtr> CardGroupDisplayWidget::getCardsMatchingGroup(QList<CardInfoPtr> cardsToSort)
{
    cardsToSort = sortCardList(cardsToSort, activeSortCriteria, Qt::SortOrder::AscendingOrder);

    QList<CardInfoPtr> activeList;
    for (const CardInfoPtr &info : cardsToSort) {
        if (info && info->getProperty(activeGroupCriteria) == cardGroupCategory) {
            activeList.append(info);
        }
    }

    return activeList;
}

QList<CardInfoPtr> CardGroupDisplayWidget::sortCardList(QList<CardInfoPtr> cardsToSort,
                                                        const QStringList properties,
                                                        Qt::SortOrder order = Qt::AscendingOrder)
{
    CardInfoComparator comparator(properties, order);
    std::sort(cardsToSort.begin(), cardsToSort.end(), comparator);

    return cardsToSort;
}

void CardGroupDisplayWidget::onActiveSortCriteriaChanged(QStringList _activeSortCriteria)
{
    if (activeSortCriteria != _activeSortCriteria) {
        activeSortCriteria = _activeSortCriteria;
        updateCardDisplays(); // Refresh display with new sorting
    }
}

void CardGroupDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card);
}

void CardGroupDisplayWidget::onHover(CardInfoPtr card)
{
    emit cardHovered(card);
}

void CardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}