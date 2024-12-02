#include "printing_selector_card_display_widget.h"

#include "../../../../deck/deck_loader.h"
#include "../../../../game/cards/card_database_manager.h"
#include "card_amount_widget.h"
#include "printing_selector_card_overlay_widget.h"
#include "set_name_and_collectors_number_display_widget.h"

#include <QGraphicsEffect>
#include <QStackedWidget>
#include <QVBoxLayout>

PrintingSelectorCardDisplayWidget::PrintingSelectorCardDisplayWidget(QWidget *parent,
                                                                     TabDeckEditor *deckEditor,
                                                                     DeckListModel *deckModel,
                                                                     QTreeView *deckView,
                                                                     QSlider *cardSizeSlider,
                                                                     CardInfoPtr rootCard,
                                                                     CardInfoPerSet setInfoForCard,
                                                                     QString &currentZone)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), cardSizeSlider(cardSizeSlider),
      rootCard(rootCard), setInfoForCard(setInfoForCard), currentZone(currentZone)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    overlayWidget = new PrintingSelectorCardOverlayWidget(this, deckEditor, deckModel, deckView, cardSizeSlider,
                                                          rootCard, setInfoForCard);

    const QString combinedSetName =
        QString(setInfoForCard.getPtr()->getLongName() + " (" + setInfoForCard.getPtr()->getShortName() + ")");
    setNameAndCollectorsNumberDisplayWidget = new SetNameAndCollectorsNumberDisplayWidget(
        this, combinedSetName, setInfoForCard.getProperty("num"), cardSizeSlider);

    layout->addWidget(overlayWidget, 0, Qt::AlignHCenter);
    layout->addWidget(setNameAndCollectorsNumberDisplayWidget, 1, Qt::AlignHCenter | Qt::AlignBottom);
}

void PrintingSelectorCardDisplayWidget::clampSetNameToPicture()
{
    if (overlayWidget != nullptr && setNameAndCollectorsNumberDisplayWidget != nullptr) {
        setNameAndCollectorsNumberDisplayWidget->setMaximumWidth(overlayWidget->width());
    }
    update();
}