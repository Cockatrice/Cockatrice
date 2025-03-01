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
                                                                   QString _activeGroupCriteria,
                                                                   QString _activeSortCriteria,
                                                                   int bannerOpacity)
    : QWidget(parent), deckListModel(_deckListModel), zoneName(_zoneName), cardGroupCategory(_cardGroupCategory),
      activeGroupCriteria(_activeGroupCriteria), activeSortCriteria(_activeSortCriteria)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setMinimumSize(QSize(0, 0));

    banner = new BannerWidget(this, cardGroupCategory, Qt::Orientation::Vertical, bannerOpacity);
    overlapWidget = new OverlapWidget(this, 80, 1, 1, Qt::Vertical, true);
    banner->setBuddy(overlapWidget);

    layout->addWidget(banner);
    layout->addWidget(overlapWidget);
    updateCardDisplays();
    connect(deckListModel, &DeckListModel::dataChanged, this, &OverlappedCardGroupDisplayWidget::updateCardDisplays);
}

void OverlappedCardGroupDisplayWidget::updateCardDisplays()
{
    QList<CardInfoPtr> cardsInZone = getCardsMatchingGroup(deckListModel->getCardsAsCardInfoPtrsForZone(zoneName));

    bool shouldBeVisible = !cardsInZone.isEmpty();
    if (shouldBeVisible != isVisible()) {
        setVisible(shouldBeVisible);
    }

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

    // Remove excess widgets
    for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
        QString name = it.key();
        QList<CardInfoPictureWithTextOverlayWidget *> &widgets = it.value();

        int neededCount = requiredCounts.value(name, 0);
        while (widgets.size() > neededCount) {
            CardInfoPictureWithTextOverlayWidget *widgetToRemove = widgets.takeLast();
            overlapWidget->layout()->removeWidget(widgetToRemove); // Remove from layout
            delete widgetToRemove;
        }
    }

    // Add missing widgets and reorder existing ones
    QList<CardInfoPictureWithTextOverlayWidget *> sortedWidgets;
    for (const CardInfoPtr &card : cardsInZone) {
        QString name = card->getName();

        if (!widgetMap[name].isEmpty()) {
            // Use existing widget
            CardInfoPictureWithTextOverlayWidget *widget = widgetMap[name].takeFirst();
            sortedWidgets.append(widget);
        } else {
            // Create new widget if needed
            auto *display = new CardInfoPictureWithTextOverlayWidget(overlapWidget, true);
            display->setCard(card);

            connect(display, &CardInfoPictureWithTextOverlayWidget::imageClicked, this,
                    &OverlappedCardGroupDisplayWidget::onClick);
            connect(display, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this,
                    &OverlappedCardGroupDisplayWidget::onHover);

            sortedWidgets.append(display);
            overlapWidget->addWidget(display);
        }
    }

    // **Reorder Widgets in Layout**
    for (CardInfoPictureWithTextOverlayWidget *widget : sortedWidgets) {
        overlapWidget->removeWidget(widget);
        overlapWidget->addWidget(widget);
    }

    for (CardInfoPictureWithTextOverlayWidget *widget : sortedWidgets) {
        widget->raise();
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
            if (info->getProperty(activeGroupCriteria) == cardGroupCategory) {
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

void OverlappedCardGroupDisplayWidget::onActiveSortCriteriaChanged(QString _activeSortCriteria)
{
    if (activeSortCriteria != _activeSortCriteria) {
        activeSortCriteria = _activeSortCriteria;
        qDebug() << "OverlappedCardGroupDisplayWidget changed the sort criteria to " << activeSortCriteria;
        updateCardDisplays(); // Refresh display with new sorting
    }
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