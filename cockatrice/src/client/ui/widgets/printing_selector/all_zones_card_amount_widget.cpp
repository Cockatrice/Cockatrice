#include "all_zones_card_amount_widget.h"

#include "../general/display/shadow_background_label.h"

#include <QTimer>

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
    buttonBoxMainboard = new CardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard,
                                              setInfoForCard, DECK_ZONE_MAIN);
    zoneLabelSideboard = new ShadowBackgroundLabel(this, tr("Sideboard"));
    buttonBoxSideboard = new CardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, rootCard,
                                              setInfoForCard, DECK_ZONE_SIDE);

    layout->addWidget(zoneLabelMainboard, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(buttonBoxMainboard, 0, Qt::AlignHCenter | Qt::AlignTop);
    layout->addSpacing(25);
    layout->addWidget(zoneLabelSideboard, 0, Qt::AlignHCenter | Qt::AlignBottom);
    layout->addWidget(buttonBoxSideboard, 0, Qt::AlignHCenter | Qt::AlignTop);

    connect(cardSizeSlider, &QSlider::valueChanged, this, &AllZonesCardAmountWidget::adjustFontSize);

    QTimer::singleShot(10, this, [this]() { adjustFontSize(this->cardSizeSlider->value()); });

    setMouseTracking(true);
}

void AllZonesCardAmountWidget::adjustFontSize(int scalePercentage)
{
    const int minFontSize = 8;      // Minimum font size
    const int maxFontSize = 32;     // Maximum font size
    const int basePercentage = 100; // Scale at 100%

    int newFontSize = minFontSize + (scalePercentage - basePercentage) * (maxFontSize - minFontSize) / (250 - 25);
    newFontSize = std::clamp(newFontSize, minFontSize, maxFontSize);

    // Update the font labels
    QFont zoneLabelFont = zoneLabelMainboard->font();
    zoneLabelFont.setPointSize(newFontSize);
    zoneLabelMainboard->setFont(zoneLabelFont);
    zoneLabelSideboard->setFont(zoneLabelFont);

    // Repaint the widget (if necessary)
    repaint();
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