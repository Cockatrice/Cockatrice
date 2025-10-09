#include "printing_selector_card_overlay_widget.h"

#include "printing_selector_card_display_widget.h"

#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/settings/cache_settings.h>
#include <utility>

/**
 * @brief Constructs a PrintingSelectorCardOverlayWidget for displaying a card overlay.
 *
 * This widget is responsible for showing the card's image and providing interactive features such
 * as a context menu and the ability to adjust the card's scale. It includes the card's image as well
 * as a widget that displays the card amounts in different zones (mainboard, sideboard, etc.).
 *
 * @param parent The parent widget for this overlay.
 * @param _deckEditor The TabDeckEditor instance for deck management.
 * @param _deckModel The DeckListModel instance providing deck data.
 * @param _deckView The QTreeView instance displaying the deck.
 * @param _cardSizeSlider The slider controlling the size of the card.
 * @param _rootCard The root card object that contains information about the card.
 */
PrintingSelectorCardOverlayWidget::PrintingSelectorCardOverlayWidget(QWidget *parent,
                                                                     AbstractTabDeckEditor *_deckEditor,
                                                                     DeckListModel *_deckModel,
                                                                     QTreeView *_deckView,
                                                                     QSlider *_cardSizeSlider,
                                                                     const ExactCard &_rootCard)
    : QWidget(parent), deckEditor(_deckEditor), deckModel(_deckModel), deckView(_deckView),
      cardSizeSlider(_cardSizeSlider), rootCard(_rootCard)
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
    cardInfoPicture->setCard(_rootCard);
    mainLayout->addWidget(cardInfoPicture);

    // Add AllZonesCardAmountWidget
    allZonesCardAmountWidget =
        new AllZonesCardAmountWidget(this, deckEditor, deckModel, deckView, cardSizeSlider, _rootCard);

    allZonesCardAmountWidget->raise(); // Ensure it's on top of the picture
    // Set initial visibility based on amounts
    if (allZonesCardAmountWidget->getMainboardAmount() > 0 || allZonesCardAmountWidget->getSideboardAmount() > 0) {
        allZonesCardAmountWidget->setVisible(true);
    } else {
        allZonesCardAmountWidget->setVisible(false);
    }

    // Attempt to cast the parent to PrintingSelectorCardDisplayWidget
    if (const auto *parentWidget = qobject_cast<PrintingSelectorCardDisplayWidget *>(parent)) {
        connect(cardInfoPicture, &CardInfoPictureWidget::cardScaleFactorChanged, parentWidget,
                &PrintingSelectorCardDisplayWidget::clampSetNameToPicture);
    }

    connect(cardSizeSlider, &QSlider::valueChanged, cardInfoPicture, &CardInfoPictureWidget::setScaleFactor);
}

/**
 * @brief Handles the mouse press event for right-clicks to show the context menu.
 *
 * If the right mouse button is pressed, a custom context menu will appear. For other mouse buttons,
 * the event is passed to the base class for default handling.
 *
 * @param event The mouse event triggered by the user.
 */
void PrintingSelectorCardOverlayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        customMenu(event->pos());
    } else {
        QWidget::mousePressEvent(event); // Pass other events to the base class
    }
}

/**
 * @brief Resizes the overlay widget to match the card's size.
 *
 * This method ensures that the amount widget matches the card's size when the overlay widget is resized.
 * It also resizes the card info picture widget to match the new size.
 *
 * @param event The resize event triggered when the widget is resized.
 */
void PrintingSelectorCardOverlayWidget::resizeEvent(QResizeEvent *event)
{
    // Ensure the amount widget matches the parent size
    QWidget::resizeEvent(event);
    if (allZonesCardAmountWidget) {
        allZonesCardAmountWidget->resize(cardInfoPicture->size());
    }
    resize(cardInfoPicture->size());
}

/**
 * @brief Handles the mouse enter event when the cursor enters the overlay widget area.
 *
 * When the cursor enters the widget, the card information is updated, and the card amount widget
 * is displayed if the amounts are zero for both the mainboard and sideboard.
 *
 * @param event The event triggered when the mouse enters the widget.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void PrintingSelectorCardOverlayWidget::enterEvent(QEnterEvent *event)
#else
void PrintingSelectorCardOverlayWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    deckEditor->updateCard(rootCard);

    // Check if either mainboard or sideboard amount is greater than 0
    if (allZonesCardAmountWidget->getMainboardAmount() > 0 || allZonesCardAmountWidget->getSideboardAmount() > 0) {
        // Don't change visibility if amounts are greater than 0
        return;
    }

    // Show the widget if amounts are 0
    allZonesCardAmountWidget->setVisible(true);
}

/**
 * @brief Handles the mouse leave event when the cursor leaves the overlay widget area.
 *
 * When the cursor leaves the widget, the card amount widget is hidden if both the mainboard and sideboard
 * amounts are zero.
 *
 * @param event The event triggered when the mouse leaves the widget.
 */
void PrintingSelectorCardOverlayWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);

    // Check if either mainboard or sideboard amount is greater than 0
    if (allZonesCardAmountWidget->getMainboardAmount() > 0 || allZonesCardAmountWidget->getSideboardAmount() > 0) {
        // Don't hide the widget if amounts are greater than 0
        return;
    }

    // Hide the widget if amounts are 0
    allZonesCardAmountWidget->setVisible(false);
}

/**
 * @brief Creates and shows a custom context menu when the right mouse button is clicked.
 *
 * The context menu includes an option to show related cards, which displays a submenu with actions
 * for each related card. When an action is triggered, the card information is updated, and the
 * printing selector is shown.
 *
 * @param point The position of the mouse when the right-click occurred.
 */
void PrintingSelectorCardOverlayWidget::customMenu(QPoint point)
{
    QMenu menu;

    auto *preferenceMenu = new QMenu(tr("Preference"));
    menu.addMenu(preferenceMenu);

    const auto &preferredProviderId =
        SettingsCache::instance().cardOverrides().getCardPreferenceOverride(rootCard.getName());
    const auto &cardProviderId = rootCard.getPrinting().getUuid();

    if (preferredProviderId.isEmpty() || preferredProviderId != cardProviderId) {
        auto *pinAction = preferenceMenu->addAction(tr("Pin Printing"));
        connect(pinAction, &QAction::triggered, this, [this] {
            SettingsCache::instance().cardOverrides().setCardPreferenceOverride(
                {rootCard.getName(), rootCard.getPrinting().getUuid()});
            emit cardPreferenceChanged();
        });
    } else {
        auto *unpinAction = preferenceMenu->addAction(tr("Unpin Printing"));
        connect(unpinAction, &QAction::triggered, this, [this] {
            SettingsCache::instance().cardOverrides().deleteCardPreferenceOverride(rootCard.getName());
            emit cardPreferenceChanged();
        });
    }

    // filling out the related cards submenu
    auto *relatedMenu = new QMenu(tr("Show Related cards"));
    menu.addMenu(relatedMenu);
    auto relatedCards = rootCard.getInfo().getAllRelatedCards();
    if (relatedCards.isEmpty()) {
        relatedMenu->setDisabled(true);
    } else {
        for (const CardRelation *rel : relatedCards) {
            const QString &relatedCardName = rel->getName();
            QAction *relatedCard = relatedMenu->addAction(relatedCardName);
            connect(relatedCard, &QAction::triggered, deckEditor, [this, relatedCardName] {
                deckEditor->updateCard(CardDatabaseManager::query()->getCard({relatedCardName}));
                deckEditor->showPrintingSelector();
            });
        }
    }
    menu.exec(this->mapToGlobal(point));
}
