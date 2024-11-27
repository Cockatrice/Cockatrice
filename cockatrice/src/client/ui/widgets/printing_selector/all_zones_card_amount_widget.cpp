#include "all_zones_card_amount_widget.h"

AllZonesCardAmountWidget::AllZonesCardAmountWidget(QWidget *parent,
                                                   TabDeckEditor *deckEditor,
                                                   DeckListModel *deckModel,
                                                   QTreeView *deckView,
                                                   CardInfoPtr rootCard,
                                                   CardInfoPerSet setInfoForCard)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), rootCard(rootCard),
      setInfoForCard(setInfoForCard)
{
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    zoneLabelMainboard = new QLabel(tr("Mainboard"), this);
    zoneLabelMainboard->setStyleSheet(R"(
    QLabel {
        background-color: rgba(0, 0, 0, 128); /* Semi-transparent black */
        color: white; /* Text color */
        font-size: 16px;
        border-radius: 5px; /* Rounded corners */
        padding: 5px; /* Padding around text */
    }
    )");
    buttonBoxMainboard =
        new CardAmountWidget(this, deckEditor, deckModel, deckView, rootCard, setInfoForCard, DECK_ZONE_MAIN);
    zoneLabelSideboard = new QLabel(tr("Sideboard"), this);
    zoneLabelSideboard->setStyleSheet(R"(
    QLabel {
        background-color: rgba(0, 0, 0, 128); /* Semi-transparent black */
        color: white; /* Text color */
        font-size: 16px;
        border-radius: 5px; /* Rounded corners */
        padding: 5px; /* Padding around text */
    }
    )");
    buttonBoxSideboard =
        new CardAmountWidget(this, deckEditor, deckModel, deckView, rootCard, setInfoForCard, DECK_ZONE_SIDE);

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