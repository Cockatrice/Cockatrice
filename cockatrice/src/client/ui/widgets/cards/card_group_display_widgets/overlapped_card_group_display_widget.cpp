#include "overlapped_card_group_display_widget.h"

#include "../../../../../deck/deck_list_model.h"
#include "../../../../../game/cards/card_database_manager.h"
#include "../../../../../utility/card_info_comparator.h"
#include "../card_info_picture_with_text_overlay_widget.h"

#include <QResizeEvent>

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
    OverlappedCardGroupDisplayWidget::updateCardDisplays();

    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, this,
            [this]() { overlapWidget->adjustMaxColumnsAndRows(); });
}

void OverlappedCardGroupDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    overlapWidget->resize(event->size());
    overlapWidget->adjustMaxColumnsAndRows();
}