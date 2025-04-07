#include "overlapped_card_group_display_widget.h"

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../utility/card_info_comparator.h"
#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>

OverlappedCardGroupDisplayWidget::OverlappedCardGroupDisplayWidget(QWidget *parent,
                                                                   DeckListModel *_deckListModel,
                                                                   QString _zoneName,
                                                                   QString _cardGroupCategory,
                                                                   QString _activeGroupCriteria,
                                                                   QStringList _activeSortCriteria,
                                                                   int bannerOpacity,
                                                                   CardSizeWidget *_cardSizeWidget)
    : CardGroupDisplayWidget(parent,
                             _deckListModel,
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
    OverlappedCardGroupDisplayWidget::updateCardDisplays();
    connect(deckListModel, &DeckListModel::dataChanged, this, &OverlappedCardGroupDisplayWidget::updateCardDisplays);
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, this,
            [this]() { overlapWidget->adjustMaxColumnsAndRows(); });
}

void OverlappedCardGroupDisplayWidget::updateCardDisplays()
{
    overlapWidget->setUpdatesEnabled(false);
    // Retrieve and sort cards
    QList<CardInfoPtr> cardsInZone = getCardsMatchingGroup(deckListModel->getCardsAsCardInfoPtrsForZone(zoneName));

    // Show or hide widget
    bool shouldBeVisible = !cardsInZone.isEmpty();
    if (shouldBeVisible != isVisible()) {
        setVisible(shouldBeVisible);
    }

    // Retrieve existing widgets
    QList<CardInfoPictureWithTextOverlayWidget *> existingWidgets =
        overlapWidget->findChildren<CardInfoPictureWithTextOverlayWidget *>();

    QHash<QString, QList<CardInfoPictureWithTextOverlayWidget *>> widgetMap;
    for (CardInfoPictureWithTextOverlayWidget *widget : existingWidgets) {
        widgetMap[widget->getInfo()->getName()].append(widget);
    }

    QList<CardInfoPictureWithTextOverlayWidget *> sortedWidgets;
    QSet<CardInfoPictureWithTextOverlayWidget *> usedWidgets;

    // Ensure widgets are ordered to match the sorted cards
    for (const CardInfoPtr &card : cardsInZone) {
        QString name = card->getName();
        CardInfoPictureWithTextOverlayWidget *widget = nullptr;

        if (!widgetMap[name].isEmpty()) {
            // Reuse an existing widget
            widget = widgetMap[name].takeFirst();
        } else {
            // Create a new widget if needed
            widget = new CardInfoPictureWithTextOverlayWidget(overlapWidget, true);
            widget->setScaleFactor(cardSizeWidget->getSlider()->value());
            widget->setCard(card);

            connect(widget, &CardInfoPictureWithTextOverlayWidget::imageClicked, this,
                    &OverlappedCardGroupDisplayWidget::onClick);
            connect(widget, &CardInfoPictureWithTextOverlayWidget::hoveredOnCard, this,
                    &OverlappedCardGroupDisplayWidget::onHover);
            connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, widget,
                    &CardInfoPictureWidget::setScaleFactor);

            overlapWidget->addWidget(widget);
        }

        // Store in sorted order
        sortedWidgets.append(widget);
        usedWidgets.insert(widget);
    }

    // Remove extra widgets
    for (CardInfoPictureWithTextOverlayWidget *widget : existingWidgets) {
        if (!usedWidgets.contains(widget)) {
            overlapWidget->layout()->removeWidget(widget);
            delete widget;
        }
    }

    // **Reorder widgets in place**
    for (int i = 0; i < sortedWidgets.size(); ++i) {
        sortedWidgets[i]->setParent(nullptr); // Temporarily detach
    }
    for (int i = 0; i < sortedWidgets.size(); ++i) {
        overlapWidget->addWidget(sortedWidgets[i]); // Reattach in correct order
    }

    // Ensure proper layering
    for (CardInfoPictureWithTextOverlayWidget *widget : sortedWidgets) {
        widget->raise();
    }

    overlapWidget->adjustMaxColumnsAndRows();
    overlapWidget->setUpdatesEnabled(true);
    overlapWidget->update();
}

void OverlappedCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    overlapWidget->resize(event->size());
    overlapWidget->adjustMaxColumnsAndRows();
}