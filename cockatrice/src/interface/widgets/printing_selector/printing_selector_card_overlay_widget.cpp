#include "printing_selector_card_overlay_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "printing_selector_card_display_widget.h"

#include <QImageReader>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QtMath>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
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

    initializePinBadge();

    // Update when this overlay emits cardPreferenceChanged or when size/scale changes
    connect(this, &PrintingSelectorCardOverlayWidget::cardPreferenceChanged, this,
            &PrintingSelectorCardOverlayWidget::updatePinBadgeVisibility);
    connect(cardSizeSlider, &QSlider::valueChanged, this, &PrintingSelectorCardOverlayWidget::updatePinBadgeVisibility);
    // initial state
    updatePinBadgeVisibility();

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
 * @brief Updates the pin badge visibility and position based on the card's pinned state.
 *
 * This method checks whether the current card printing is pinned and updates the
 * pin badge accordingly. If the card is pinned, the badge is made visible and positioned in the
 * top-right corner of the card image with appropriate margins. If the card is not pinned, the
 * badge is hidden.
 *
 * The method is called whenever the card preference changes or the card size is adjusted via
 * the slider to ensure the badge remains properly positioned.
 */
void PrintingSelectorCardOverlayWidget::updatePinBadgeVisibility()
{
    if (!pinBadge || !cardInfoPicture)
        return;

    // Query the persisted preference override to decide whether this printing is pinned.
    const auto &preferredProviderId =
        SettingsCache::instance().cardOverrides().getCardPreferenceOverride(rootCard.getName());
    const auto &cardProviderId = rootCard.getPrinting().getUuid();
    const bool isPinned = (!preferredProviderId.isEmpty() && preferredProviderId == cardProviderId);

    // Toggle the badge once; the pixmap was already rasterized in initializePinBadge().
    pinBadge->setVisible(isPinned);

    if (isPinned) {
        // Keep a small margin that scales with the card size to avoid obscuring stuff.
        const int margin = qMax(3, int(cardInfoPicture->width() * 0.03));
        int x = qMax(0, cardInfoPicture->width() - pinBadge->width() - margin);
        int y = margin * 3;
        pinBadge->move(x, y);
        pinBadge->raise();
    }
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

/**
 * @brief Initializes the pin badge overlay and loads its icon with DPI-aware rasterization.
 *
 * The icon is rasterized once using the label's device pixel ratio so it stays crisp on HiDPI
 * displays. The resulting pixmap is cached on the QLabel and simply shown/hidden when needed.
 * If the SVG cannot be read, a textual fallback badge is created instead.
 */
void PrintingSelectorCardOverlayWidget::initializePinBadge()
{
    if (!pinBadge) {
        // construct the overlay label once
        pinBadge = new QLabel(this);
        pinBadge->setObjectName(QStringLiteral("printingSelectorPinBadge"));
    } else {
        // Clear any previous pixmap / style in case we reinitialize for a DPR change.
        pinBadge->clear();
        pinBadge->setStyleSheet(QString());
    }

    bool pinLoaded = false;
    QImageReader pinReader(QStringLiteral("theme:icons/pin"));

    if (pinReader.canRead()) {
        // Rasterize a 64×64 logical icon so it has a consistent size regardless of card scaling.
        const QSize targetSize(64, 64);
        const qreal dpr = pinBadge->devicePixelRatioF();
        const QSize rasterSize(qMax(1, qCeil(targetSize.width() * dpr)), qMax(1, qCeil(targetSize.height() * dpr)));
        pinReader.setScaledSize(rasterSize);
        const QImage pinImage = pinReader.read();
        if (!pinImage.isNull()) {
            // Tag the pixmap with the same DPR so Qt renders it at the correct physical size.
            QPixmap pinPix = QPixmap::fromImage(pinImage);
            pinPix.setDevicePixelRatio(dpr);
            pinBadge->setPixmap(pinPix);
            pinBadge->setFixedSize(targetSize);
            pinBadge->setStyleSheet(QStringLiteral("background: transparent;"));
            pinLoaded = true;
        }
    }

    if (!pinLoaded) {
        // Fall back to a text badge when the SVG cannot be decoded (e.g., missing theme resource).
        pinBadge->setText(QStringLiteral("PIN"));
        pinBadge->setAlignment(Qt::AlignCenter);
        pinBadge->setFixedSize(24, 12);
        pinBadge->setStyleSheet(QStringLiteral("background: yellow; color: black; border: 1px solid red;"));
    }

    // The overlay is mouse-transparent and hidden until we know the card is pinned.
    pinBadge->setAttribute(Qt::WA_TransparentForMouseEvents);
    pinBadge->setVisible(false);
    pinBadge->raise();
}
