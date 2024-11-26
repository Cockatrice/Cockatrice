#include "printing_selector_card_overlay_widget.h"

#include "../../../../game/cards/card_database_manager.h"
#include "printing_selector_card_display_widget.h"

#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>

PrintingSelectorCardOverlayWidget::PrintingSelectorCardOverlayWidget(QWidget *parent,
                                                                     TabDeckEditor *deckEditor,
                                                                     DeckListModel *deckModel,
                                                                     QTreeView *deckView,
                                                                     QSlider *cardSizeSlider,
                                                                     CardInfoPtr rootCard,
                                                                     CardInfoPerSet setInfoForCard)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView), rootCard(rootCard),
      setInfoForCard(setInfoForCard)
{
    // Set up the main layout
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    // Add CardInfoPictureWidget
    cardInfoPicture = new CardInfoPictureWidget(this);
    cardInfoPicture->setMinimumSize(0, 0);
    cardInfoPicture->setScaleFactor(cardSizeSlider->value());
    setCard = CardDatabaseManager::getInstance()->getCardByNameAndProviderId(rootCard->getName(),
                                                                             setInfoForCard.getProperty("uuid"));
    cardInfoPicture->setCard(setCard);
    mainLayout->addWidget(cardInfoPicture);

    // Add AllZonesCardAmountWidget
    allZonesCardAmountWidget =
        new AllZonesCardAmountWidget(this, deckEditor, deckModel, deckView, setCard, setInfoForCard);

    allZonesCardAmountWidget->raise(); // Ensure it's on top of the picture
    allZonesCardAmountWidget->setVisible(false);

    // Attempt to cast the parent to PrintingSelectorCardDisplayWidget
    if (const auto *parentWidget = qobject_cast<PrintingSelectorCardDisplayWidget *>(parent)) {
        connect(cardInfoPicture, &CardInfoPictureWidget::cardScaleFactorChanged, parentWidget,
                &PrintingSelectorCardDisplayWidget::clampSetNameToPicture);
    }

    connect(cardSizeSlider, &QSlider::valueChanged, cardInfoPicture, &CardInfoPictureWidget::setScaleFactor);
}

void PrintingSelectorCardOverlayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        customMenu(event->pos());
    } else {
        QWidget::mousePressEvent(event); // Pass other events to the base class
    }
}

void PrintingSelectorCardOverlayWidget::resizeEvent(QResizeEvent *event)
{
    // Ensure the amount widget matches the parent size
    QWidget::resizeEvent(event);
    if (allZonesCardAmountWidget) {
        allZonesCardAmountWidget->resize(cardInfoPicture->size());
    }
    resize(cardInfoPicture->size());
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void PrintingSelectorCardOverlayWidget::enterEvent(QEnterEvent *event)
#else
void PrintingSelectorCardOverlayWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    allZonesCardAmountWidget->setVisible(true);
    deckEditor->updateCardInfo(setCard);
}

void PrintingSelectorCardOverlayWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    allZonesCardAmountWidget->setVisible(false);
}

void PrintingSelectorCardOverlayWidget::customMenu(QPoint point)
{
    QMenu menu;
    // filling out the related cards submenu
    auto *relatedMenu = new QMenu(tr("Show Related cards"));
    menu.addMenu(relatedMenu);
    auto relatedCards = rootCard->getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        relatedMenu->setDisabled(true);
    } else {
        for (const CardRelation *rel : relatedCards) {
            const QString &relatedCardName = rel->getName();
            QAction *relatedCard = relatedMenu->addAction(relatedCardName);
            connect(relatedCard, &QAction::triggered, deckEditor, [this, relatedCardName] {
                deckEditor->updateCardInfo(CardDatabaseManager::getInstance()->getCard(relatedCardName));
                deckEditor->showPrintingSelector();
            });
        }
    }
    menu.exec(this->mapToGlobal(point));
}
