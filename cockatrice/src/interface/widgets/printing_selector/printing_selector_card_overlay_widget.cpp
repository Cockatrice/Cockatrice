#include "printing_selector_card_overlay_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../../card_picture_loader/card_picture_loader.h"
#include "../cards/card_info_picture_widget.h"
#include "printing_selector_card_display_widget.h"

#include <QApplication>
#include <QFileDialog>
#include <QImageReader>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmapCache>
#include <QScreen>
#include <QVBoxLayout>
#include <QtMath>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/card/relation/card_relation.h>
#include <libcockatrice/settings/card_override_settings.h>
#include <utility>

namespace
{
/**
 * @brief Places the preview beside the highlighted action inside the given screen.
 *
 * Side-aware: hugs the side of the action that has room, aligned with its row, then
 * clamps every edge so the preview always lands fully on-screen on first show.
 */
QPoint
previewPositionNear(const QRect &actionRect, const QSize &labelSize, const QRect &screenGeometry, int previewOffset)
{
    const bool rightFits = actionRect.right() + previewOffset + labelSize.width() <= screenGeometry.right();
    const bool leftFits = actionRect.left() - previewOffset - labelSize.width() >= screenGeometry.left();

    int x;
    if (rightFits) {
        x = actionRect.right() + previewOffset;
    } else if (leftFits) {
        x = actionRect.left() - previewOffset - labelSize.width();
    } else {
        x = actionRect.left();
    }
    x = qMax(screenGeometry.left(), x);
    x = qMin(screenGeometry.right() - labelSize.width() + 1, x);

    const bool belowFits = actionRect.bottom() + previewOffset + labelSize.height() <= screenGeometry.bottom();
    const bool aboveFits = actionRect.top() - previewOffset - labelSize.height() >= screenGeometry.top();

    int y;
    if (belowFits) {
        y = actionRect.bottom() + previewOffset;
    } else if (aboveFits) {
        y = actionRect.top() - previewOffset - labelSize.height();
    } else {
        y = actionRect.top();
    }
    y = qMax(screenGeometry.top(), y);
    y = qMin(screenGeometry.bottom() - labelSize.height() + 1, y);

    return {x, y};
}
} // namespace

/**
 * @brief Constructs a PrintingSelectorCardOverlayWidget for displaying a card overlay.
 *
 * This widget is responsible for showing the card's image and providing interactive features such
 * as a context menu and the ability to adjust the card's scale. It includes the card's image as well
 * as a widget that displays the card amounts in different zones (mainboard, sideboard, etc.).
 *
 * @param parent The parent widget for this overlay.
 * @param _deckEditor The TabDeckEditor instance for deck management.
 * @param deckStateManager The DeckStateManager instance providing deck data.
 * @param cardSizeSlider The slider controlling the size of the card.
 * @param _rootCard The root card object that contains information about the card.
 */
PrintingSelectorCardOverlayWidget::PrintingSelectorCardOverlayWidget(QWidget *parent,
                                                                     AbstractTabDeckEditor *_deckEditor,
                                                                     DeckStateManager *deckStateManager,
                                                                     QSlider *cardSizeSlider,
                                                                     const ExactCard &_rootCard)
    : QWidget(parent), deckEditor(_deckEditor), rootCard(_rootCard)
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

    // Parent the preview to this overlay so it is destroyed with it (Qt::ToolTip keeps
    // it a frameless, non-activating top-level window despite the parent).
    cardOverridePreviewLabel = new QLabel(this, Qt::ToolTip);
    cardOverridePreviewLabel->setWindowFlag(Qt::FramelessWindowHint);
    cardOverridePreviewLabel->setAttribute(Qt::WA_ShowWithoutActivating);
    cardOverridePreviewLabel->setScaledContents(true);
    cardOverridePreviewLabel->hide();

    // While the preview is visible, keep it honest: when the hovered printing's art
    // resolves (all alternate printings share the root card's CardInfo), redraw it in place.
    if (rootCard.getCardPtr()) {
        connect(rootCard.getCardPtr().data(), &CardInfo::pixmapUpdated, this, [this] {
            if (cardOverridePreviewLabel->isVisible()) {
                refreshPreview();
            }
        });
    }

    // Alt-Tab / app-inactive must not strand the floating preview.
    connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            hidePreview();
        }
    });

    // Update when this overlay emits cardPreferenceChanged or when size/scale changes
    connect(this, &PrintingSelectorCardOverlayWidget::cardPreferenceChanged, this,
            &PrintingSelectorCardOverlayWidget::updatePinBadgeVisibility);
    connect(cardSizeSlider, &QSlider::valueChanged, this, &PrintingSelectorCardOverlayWidget::updatePinBadgeVisibility);
    // initial state
    updatePinBadgeVisibility();

    // Add AllZonesCardAmountWidget
    allZonesCardAmountWidget = new AllZonesCardAmountWidget(this, deckStateManager, cardSizeSlider, _rootCard);

    allZonesCardAmountWidget->raise(); // Ensure it's on top of the picture

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
 * When the cursor enters the widget, the card amount widget becomes visible regardless of whether the amounts are zero.
 *
 * @param event The event triggered when the mouse enters the widget.
 */
void PrintingSelectorCardOverlayWidget::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    deckEditor->updateCard(rootCard);
    updateVisibility();
}

void PrintingSelectorCardOverlayWidget::updateCardAmounts(int mainboardAmount,
                                                          int sideboardAmount,
                                                          int tokensboardAmount)
{
    allZonesCardAmountWidget->setAmounts(mainboardAmount, sideboardAmount, tokensboardAmount);
    updateVisibility();
}

/**
 * @brief Sets the visibility of the widgets depending on the amounts and whether the mouse is hovering over.
 */
void PrintingSelectorCardOverlayWidget::updateVisibility()
{
    if (allZonesCardAmountWidget->isNonZero() || underMouse()) {
        allZonesCardAmountWidget->setVisible(true);
    } else {
        allZonesCardAmountWidget->setVisible(false);
    }
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
    if (!pinBadge || !cardInfoPicture) {
        return;
    }

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
 * When the cursor leaves the widget, the card amount widget is hidden if all of the mainboard, sideboard, and
 * tokensboard amounts are zero.
 *
 * @param event The event triggered when the mouse leaves the widget.
 */
void PrintingSelectorCardOverlayWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    updateVisibility();
}

/**
 * @brief Creates and shows the card-overlay context menu.
 *
 * The menu includes the card art preference (Pin/Unpin Printing), the Image Overrides
 * submenu (Load Custom Image, Clear Custom Image, and one entry per alternate printing with a
 * live preview), and the Show Related cards submenu.
 *
 * @param point The local position the menu should pop at.
 */
void PrintingSelectorCardOverlayWidget::customMenu(QPoint point)
{
    QMenu menu;

    hidePreview(); // Clear any preview state left over from a previous menu run.

    // Submenus are owned by the stack-allocated top-level menu (addMenu() does not
    // transfer ownership).
    auto *preferenceMenu = new QMenu(tr("Preference"), &menu);
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

    menu.addSeparator();

    auto *overrideMenu = new QMenu(tr("Image Overrides"), &menu);

    auto *loadCustomAction = overrideMenu->addAction(tr("Load Custom Image..."));
    auto *clearOverrideAction = overrideMenu->addAction(tr("Clear Custom Image"));

    // Nothing to clear on a card that has no local override yet.
    clearOverrideAction->setEnabled(CardPictureLoader::hasLocalOverrides(rootCard));

    overrideMenu->addSeparator();

    const auto &allSets = rootCard.getInfo().getSets();

    for (const auto &set : allSets) {
        for (const auto &printing : set) {
            if (printing == rootCard.getPrinting()) {
                continue;
            }

            // The submenu is already scoped to this card, so the rows lead with set +
            // collector; only printings with a distinct display name add their own name.
            const CardSetPtr cardSet = printing.getSet();
            if (!cardSet) {
                continue;
            }

            QString label = tr("%1 %2").arg(cardSet->getCorrectedShortName(), printing.getProperty("num"));

            auto *action = overrideMenu->addAction(label);

            ExactCard overrideCard(rootCard.getCardPtr(), printing);
            action->setData(QVariant::fromValue(overrideCard));

            connect(action, &QAction::triggered, this, [this, overrideCard]() {
                CardPictureLoader::getInstance().installPrintingOverride(rootCard, overrideCard);
                QPixmapCache::clear();
                rootCard.emitPixmapUpdated(); // refresh the overlay art in place, like the other paths
            });
        }
    }

    connect(clearOverrideAction, &QAction::triggered, this, [this]() {
        CardPictureLoader::deleteAllLocalOverrides(rootCard);
        QPixmapCache::clear();
        rootCard.emitPixmapUpdated(); // force UI refresh
    });

    connect(loadCustomAction, &QAction::triggered, this, &PrintingSelectorCardOverlayWidget::loadCustomImage);

    connect(overrideMenu, &QMenu::hovered, this, &PrintingSelectorCardOverlayWidget::showPreviewForAction);
    connect(overrideMenu, &QMenu::aboutToHide, this, &PrintingSelectorCardOverlayWidget::hidePreview);
    connect(overrideMenu, &QMenu::triggered, this, &PrintingSelectorCardOverlayWidget::hidePreview);

    menu.addMenu(overrideMenu);

    menu.addSeparator();

    // filling out the related cards submenu
    auto *relatedMenu = new QMenu(tr("Show Related cards"), &menu);
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
    // The preview anchors itself to this popup's global geometry while it is open, so the
    // pointer must stay valid for the whole exec() and be dropped before the stack unwinds.
    previewSourceMenu = overrideMenu;
    menu.exec(this->mapToGlobal(point));
    previewSourceMenu = nullptr;
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

/**
 * @brief Asks for an image file and installs it as the card's custom art.
 *
 * Unreadable files answer with a visible warning instead of a silent no-op.
 */
void PrintingSelectorCardOverlayWidget::loadCustomImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Card Image"), QString(),
                                                    tr("Images (*.png *.jpg *.jpeg *.webp)"));

    if (filePath.isEmpty()) {
        return;
    }

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        // No silent paths: a file that cannot be read answers visibly instead of a no-op.
        QMessageBox::warning(this, tr("Load Custom Image"), tr("The selected file could not be read as an image."));
        return;
    }

    CardPictureLoader::getInstance().saveCardImageToLocalStorage(rootCard, pixmap, true);

    QPixmapCache::clear();
    rootCard.emitPixmapUpdated();
}

/**
 * @brief Shows the hover preview for a highlighted printing entry in the Image Overrides submenu.
 *
 * QMenu::hovered fires on keyboard highlight too, so the preview appears when arrows walk
 * onto a printing entry, not only under the mouse.
 *
 * Non-printing entries (e.g., Load Custom Image, Clear Custom Image) hide the preview.
 *
 * @param action The action that was highlighted.
 */
void PrintingSelectorCardOverlayWidget::showPreviewForAction(QAction *action)
{
    if (!action) {
        hidePreview();
        return;
    }

    const QVariant data = action->data();

    if (!data.canConvert<ExactCard>()) {
        hidePreview();
        return;
    }

    const ExactCard previewCard = qvariant_cast<ExactCard>(data);
    if (previewCard.isEmpty()) {
        hidePreview();
        return;
    }

    hoveredOverrideCard = previewCard;
    hoveredOverrideAction = action;
    refreshPreview();
}

/**
 * @brief Renders the hover preview for the currently highlighted printing.
 *
 * The preview shows the loading placeholder while the art is pending and swaps in the real
 * art when it resolves. The label is positioned against its already-resized geometry so the
 * first-ever show at the screen's edges stays fully on-screen.
 */
void PrintingSelectorCardOverlayWidget::refreshPreview()
{
    if (hoveredOverrideCard.isEmpty()) {
        hidePreview();
        return;
    }

    constexpr QSize previewSize(240, 336);
    constexpr int previewOffset = 20;

    QPixmap pixmap;
    CardPictureLoader::getPixmap(pixmap, hoveredOverrideCard, previewSize);

    if (pixmap.isNull()) {
        // Keep the preview honest while loading: show the loading placeholder instead of a void.
        // Fetch at the logical size and let the label scale it, so the placeholder matches the
        // real art's footprint rather than doubling on HiDPI displays.
        CardPictureLoader::getCardBackLoadingInProgressPixmap(pixmap, previewSize);
    }

    cardOverridePreviewLabel->setPixmap(pixmap);
    // QPixmap::size() is physical pixels; the label layout must use the device-independent size
    // so the preview keeps a constant footprint across DPI settings (QScreen geometry is logical).
    const QSize labelSize = pixmap.deviceIndependentSize().toSize();
    cardOverridePreviewLabel->resize(labelSize);

    // Anchor the preview to the walked submenu popup rather than QCursor::pos(), which is idle
    // under keyboard-only operation: a keyboard-highlighted row must preview at the same place as
    // a hovered one. The mouse path is unchanged in effect — the popup sits under the cursor, so
    // the preview stays beside the row in both modalities.
    const QMenu *popup = previewSourceMenu;
    if (!popup || !popup->isVisible() || !hoveredOverrideAction) {
        hidePreview();
        return;
    }

    const QRect popupGeometry = popup->geometry();
    const QRect actionRectLocal = popup->actionGeometry(hoveredOverrideAction);
    const QRect actionRect(popupGeometry.topLeft() + actionRectLocal.topLeft(), actionRectLocal.size());

    QScreen *screen = QGuiApplication::screenAt(popupGeometry.center());
    if (!screen) {
        hidePreview();
        return;
    }
    const QRect &screenGeometry = screen->geometry();

    cardOverridePreviewLabel->move(previewPositionNear(actionRect, labelSize, screenGeometry, previewOffset));
    cardOverridePreviewLabel->show();
}

/**
 * @brief Hides the hover preview and forgets the currently highlighted printing.
 */
void PrintingSelectorCardOverlayWidget::hidePreview()
{
    hoveredOverrideCard = ExactCard();
    hoveredOverrideAction = nullptr;

    if (cardOverridePreviewLabel) {
        cardOverridePreviewLabel->hide();
    }
}
