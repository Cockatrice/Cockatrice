#include "overlapped_card_group_display_widget.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database_manager.h"
#include "../../../../utility/card_info_comparator.h"
#include "card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>

OverlappedCardGroupDisplayWidget::OverlappedCardGroupDisplayWidget(QWidget *parent,
                                                                   DeckListModel *_deckListModel,
                                                                   QString _zoneName,
                                                                   QString _cardGroupCategory,
                                                                   QString _activeSortCriteria,
                                                                   int bannerOpacity)
    : QWidget(parent), deckListModel(_deckListModel), zoneName(_zoneName), cardGroupCategory(_cardGroupCategory),
      activeSortCriteria(_activeSortCriteria)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setMinimumSize(QSize(0, 0));

    banner = new BannerWidget(this, cardGroupCategory, Qt::Orientation::Vertical, bannerOpacity);
    overlapWidget = new OverlapWidget(this, 80, 1, 1, Qt::Vertical, true);

    layout->addWidget(banner);
    layout->addWidget(overlapWidget);
    updateCardDisplays();
    connect(deckListModel, &DeckListModel::dataChanged, this, &OverlappedCardGroupDisplayWidget::updateCardDisplays);
}

void OverlappedCardGroupDisplayWidget::updateCardDisplays()
{
    QList<CardInfoPtr> cardsInZone = getCardsMatchingGroup(deckListModel->getCardsAsCardInfoPtrsForZone(zoneName));

    QList<CardInfoPictureWithTextOverlayWidget *> existingWidgets =
        overlapWidget->findChildren<CardInfoPictureWithTextOverlayWidget *>();

    QHash<QString, int> requiredCounts;
    for (const CardInfoPtr &card : cardsInZone) {
        requiredCounts[card->getName()]++;
    }

    QHash<QString, QList<CardInfoPictureWithTextOverlayWidget *>> widgetMap;
    for (CardInfoPictureWithTextOverlayWidget *widget : existingWidgets) {
        widgetMap[widget->getInfo()->getName()].append(widget);
    }

    for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
        QString name = it.key();
        QList<CardInfoPictureWithTextOverlayWidget *> &widgets = it.value();

        int neededCount = requiredCounts.value(name, 0);
        while (widgets.size() > neededCount) {
            CardInfoPictureWithTextOverlayWidget *widgetToRemove = widgets.takeLast();
            overlapWidget->removeWidget(widgetToRemove);
            delete widgetToRemove;
        }
    }

    for (auto it = requiredCounts.begin(); it != requiredCounts.end(); ++it) {
        QString name = it.key();
        int missingCount = it.value() - widgetMap[name].size();

        for (int i = 0; i < missingCount; ++i) {
            auto *display = new CardInfoPictureWithTextOverlayWidget(overlapWidget, true);

            auto cardIt = std::find_if(cardsInZone.begin(), cardsInZone.end(),
                                       [&](const CardInfoPtr &c) { return c->getName() == name; });

            if (cardIt != cardsInZone.end()) {
                display->setCard(*cardIt);
            }

            connect(display, &CardInfoPictureWithTextOverlayWidget::imageClicked, this,
                    &OverlappedCardGroupDisplayWidget::onClick);
            connect(display, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this,
                    &OverlappedCardGroupDisplayWidget::onHover);

            overlapWidget->addWidget(display);
        }
    }

    overlapWidget->adjustMaxColumnsAndRows();
}

QList<CardInfoPtr> OverlappedCardGroupDisplayWidget::getCardsMatchingGroup(QList<CardInfoPtr> *cardsToSort)
{
    QList<CardInfoPtr> activeList;

    QStringList sortCriteria;
    sortCriteria.append(activeSortCriteria);
    sortCardList(cardsToSort, sortCriteria, Qt::SortOrder::AscendingOrder);

    for (int i = 0; i < cardsToSort->size(); ++i) {
        CardInfoPtr info = cardsToSort->at(i);
        if (info) {
            if (info->getProperty(activeSortCriteria) == cardGroupCategory) {
                activeList.append(info);
            }
        } else {
            qDebug() << "Card not found in database!";
        }
    }

    return activeList;
}

void OverlappedCardGroupDisplayWidget::sortCardList(QList<CardInfoPtr> *cardsToSort,
                                                    const QStringList properties,
                                                    Qt::SortOrder order = Qt::AscendingOrder)
{
    CardInfoComparator comparator(properties, order);
    std::sort(cardsToSort->begin(), cardsToSort->end(), comparator);
}

void OverlappedCardGroupDisplayWidget::onClick(QMouseEvent *event, CardInfoPictureWithTextOverlayWidget *card)
{
    emit cardClicked(event, card);
}

void OverlappedCardGroupDisplayWidget::onHover(CardInfoPtr card)
{
    emit cardHovered(card);
}

void OverlappedCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    overlapWidget->resize(event->size());
    overlapWidget->adjustMaxColumnsAndRows();
}