#include "all_zones_card_amount_widget.h"

#include "../general/display/shadow_background_label.h"

AllZonesCardAmountWidget::AllZonesCardAmountWidget(QWidget *parent,
                                                   TabDeckEditor *deckEditor,
                                                   DeckListModel *deckModel,
                                                   QTreeView *deckView,
                                                   QSlider *cardSizeSlider,
                                                   CardInfoPtr rootCard,
                                                   CardInfoPerSet setInfoForCard)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), cardSizeSlider(cardSizeSlider),
      rootCard(rootCard), setInfoForCard(setInfoForCard)
{
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setContentsMargins(5, 5, 5, 5); // Padding around the text

    zoneLabelMainboard = new ShadowBackgroundLabel(this, tr("Mainboard"));
    buttonBoxMainboard =
        new CardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard, setInfoForCard, DECK_ZONE_MAIN);
    zoneLabelSideboard = new ShadowBackgroundLabel(this, tr("Sideboard"));
    buttonBoxSideboard =
        new CardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard, setInfoForCard, DECK_ZONE_SIDE);

    layout->addWidget(zoneLabelMainboard, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(buttonBoxMainboard, 0, Qt::AlignHCenter | Qt::AlignTop);
    layout->addSpacing(25);
    layout->addWidget(zoneLabelSideboard, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(buttonBoxSideboard, 0, Qt::AlignHCenter | Qt::AlignTop);

    setMouseTracking(true);
}

int AllZonesCardAmountWidget::getMainboardAmount()
{
    return buttonBoxMainboard->countCardsInZone(DECK_ZONE_MAIN);
}

int AllZonesCardAmountWidget::getSideboardAmount()
{
    return buttonBoxSideboard->countCardsInZone(DECK_ZONE_SIDE);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void AllZonesCardAmountWidget::enterEvent(QEnterEvent *event)
#else
void AllZonesCardAmountWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    update();
}