#include "view_zone_widget.h"

#include "../../client/settings/cache_settings.h"
#include "../../filters/syntax_help.h"
#include "../../interface/pixel_map_generator.h"
#include "../board/card_item.h"
#include "../game_scene.h"
#include "../player/player.h"
#include "../player/player_actions.h"
#include "view_zone.h"

#include <QCheckBox>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <libcockatrice/protocol/pb/command_shuffle.pb.h>

namespace {
constexpr qreal kTitleBarHeight = 24.0;
constexpr qreal kMinVisibleWidth = 100.0;
}

/**
 * @param _player the player the cards were revealed to.
 * @param _origZone the zone the cards were revealed from.
 * @param numberCards num of cards to reveal from the zone. Ex: scry the top 3 cards.
 * Pass in a negative number to reveal the entire zone.
 * -1 specifically will give the option to shuffle the zone upon closing the window.
 * @param _revealZone if false, the cards will be face down.
 * @param _writeableRevealZone whether the player can interact with the revealed cards.
 */
ZoneViewWidget::ZoneViewWidget(Player *_player,
                               CardZoneLogic *_origZone,
                               int numberCards,
                               bool _revealZone,
                               bool _writeableRevealZone,
                               const QList<const ServerInfo_Card *> &cardList,
                               bool _isReversed)
    : QGraphicsWidget(0, Qt::Window), canBeShuffled(_origZone->getIsShufflable()), player(_player)
{
    setAcceptHoverEvents(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setZValue(2000000006);
    setFlag(ItemIgnoresTransformations);

    QGraphicsLinearLayout *vbox = new QGraphicsLinearLayout(Qt::Vertical);
    vbox->setSpacing(2);

    // If the number is < 0, then it means that we can give the option to make the area sorted
    if (numberCards < 0) {
        // search edit
        searchEdit.setFocusPolicy(Qt::ClickFocus);
        searchEdit.setPlaceholderText(tr("Search by card name (or search expressions)"));
        searchEdit.setClearButtonEnabled(true);
        searchEdit.addAction(loadColorAdjustedPixmap("theme:icons/search"), QLineEdit::LeadingPosition);
        auto help = searchEdit.addAction(QPixmap("theme:icons/info"), QLineEdit::TrailingPosition);

        connect(help, &QAction::triggered, this, [this] { createSearchSyntaxHelpWindow(&searchEdit); });

        if (SettingsCache::instance().getFocusCardViewSearchBar()) {
            this->setActive(true);
            searchEdit.setFocus();
        }

        QGraphicsProxyWidget *searchEditProxy = new QGraphicsProxyWidget;
        searchEditProxy->setWidget(&searchEdit);
        searchEditProxy->setZValue(2000000007);
        vbox->addItem(searchEditProxy);

        // top row
        QGraphicsLinearLayout *hTopRow = new QGraphicsLinearLayout(Qt::Horizontal);

        // groupBy options
        QGraphicsProxyWidget *groupBySelectorProxy = new QGraphicsProxyWidget;
        groupBySelectorProxy->setWidget(&groupBySelector);
        groupBySelectorProxy->setZValue(2000000008);
        hTopRow->addItem(groupBySelectorProxy);

        // sortBy options
        QGraphicsProxyWidget *sortBySelectorProxy = new QGraphicsProxyWidget;
        sortBySelectorProxy->setWidget(&sortBySelector);
        sortBySelectorProxy->setZValue(2000000007);
        hTopRow->addItem(sortBySelectorProxy);

        vbox->addItem(hTopRow);

        // line
        QGraphicsProxyWidget *lineProxy = new QGraphicsProxyWidget;
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lineProxy->setWidget(line);
        vbox->addItem(lineProxy);

        // bottom row
        QGraphicsLinearLayout *hBottomRow = new QGraphicsLinearLayout(Qt::Horizontal);

        // pile view options
        QGraphicsProxyWidget *pileViewProxy = new QGraphicsProxyWidget;
        pileViewProxy->setWidget(&pileViewCheckBox);
        hBottomRow->addItem(pileViewProxy);

        // shuffle options
        if (_origZone->getIsShufflable() && numberCards == -1) {
            shuffleCheckBox.setChecked(true);
            QGraphicsProxyWidget *shuffleProxy = new QGraphicsProxyWidget;
            shuffleProxy->setWidget(&shuffleCheckBox);
            hBottomRow->addItem(shuffleProxy);
        }

        vbox->addItem(hBottomRow);
    }

    extraHeight = vbox->sizeHint(Qt::PreferredSize).height();

    QGraphicsLinearLayout *zoneHBox = new QGraphicsLinearLayout(Qt::Horizontal);

    zoneContainer = new QGraphicsWidget(this);
    zoneContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    zoneContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    zoneHBox->addItem(zoneContainer);

    scrollBar = new QScrollBar(Qt::Vertical);
    scrollBar->setMinimum(0);
    scrollBar->setSingleStep(20);
    scrollBar->setPageStep(200);
    connect(scrollBar, &QScrollBar::valueChanged, this, &ZoneViewWidget::handleScrollBarChange);
    scrollBarProxy = new ScrollableGraphicsProxyWidget;
    scrollBarProxy->setWidget(scrollBar);
    zoneHBox->addItem(scrollBarProxy);

    vbox->addItem(zoneHBox);

    zone = new ZoneViewZone(new ZoneViewZoneLogic(player, _origZone, numberCards, _revealZone, _writeableRevealZone,
                                                  _isReversed, zoneContainer),
                            zoneContainer);
    connect(zone, &ZoneViewZone::wheelEventReceived, scrollBarProxy, &ScrollableGraphicsProxyWidget::recieveWheelEvent);

    retranslateUi();

    // only wire up sort options after creating ZoneViewZone, since it segfaults otherwise.
    if (numberCards < 0) {
        connect(&groupBySelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &ZoneViewWidget::processGroupBy);
        connect(&sortBySelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                &ZoneViewWidget::processSortBy);
        connect(&pileViewCheckBox, &QCheckBox::QT_STATE_CHANGED, this, &ZoneViewWidget::processSetPileView);
        groupBySelector.setCurrentIndex(SettingsCache::instance().getZoneViewGroupByIndex());
        sortBySelector.setCurrentIndex(SettingsCache::instance().getZoneViewSortByIndex());
        pileViewCheckBox.setChecked(SettingsCache::instance().getZoneViewPileView());

        if (CardList::NoSort == static_cast<CardList::SortOption>(groupBySelector.currentData().toInt())) {
            pileViewCheckBox.setEnabled(false);
        }

        connect(&searchEdit, &QLineEdit::textChanged, zone, &ZoneViewZone::setFilterString);
    }

    setLayout(vbox);

    connect(zone, &ZoneViewZone::optimumRectChanged, this, [this] { resizeToZoneContents(); });
    connect(zone, &ZoneViewZone::closed, this, &ZoneViewWidget::zoneDeleted);
    zone->initializeCards(cardList);

    // QLabel sizes aren't taken into account until the widget is rendered.
    // Force refresh after 1ms to fix glitchy rendering with long QLabels.
    auto *lastResizeBeforeVisibleTimer = new QTimer(this);
    connect(lastResizeBeforeVisibleTimer, &QTimer::timeout, this, [=, this] {
        resizeToZoneContents();
        disconnect(lastResizeBeforeVisibleTimer);
        lastResizeBeforeVisibleTimer->deleteLater();
    });
    lastResizeBeforeVisibleTimer->setSingleShot(true);
    lastResizeBeforeVisibleTimer->start(1);
}

void ZoneViewWidget::processGroupBy(int index)
{
    auto option = static_cast<CardList::SortOption>(groupBySelector.itemData(index).toInt());
    SettingsCache::instance().setZoneViewGroupByIndex(index);
    zone->setGroupBy(option);

    // disable pile view checkbox if we're not grouping by anything
    pileViewCheckBox.setEnabled(option != CardList::NoSort);

    // reset sortBy if it has the same value as groupBy
    if (option != CardList::NoSort &&
        option == static_cast<CardList::SortOption>(sortBySelector.currentData().toInt())) {
        sortBySelector.setCurrentIndex(1); // set to SortByName
    }
}

void ZoneViewWidget::processSortBy(int index)
{
    auto option = static_cast<CardList::SortOption>(sortBySelector.itemData(index).toInt());

    // set to SortByName instead if it has the same value as groupBy
    if (option != CardList::NoSort &&
        option == static_cast<CardList::SortOption>(groupBySelector.currentData().toInt())) {
        sortBySelector.setCurrentIndex(1); // set to SortByName
        return;
    }

    SettingsCache::instance().setZoneViewSortByIndex(index);
    zone->setSortBy(option);
}

void ZoneViewWidget::processSetPileView(QT_STATE_CHANGED_T value)
{
    SettingsCache::instance().setZoneViewPileView(value);
    zone->setPileView(value);
}

void ZoneViewWidget::retranslateUi()
{
    setWindowTitle(zone->getLogic()->getTranslatedName(false, CaseNominative));

    { // We can't change the strings after they're put into the QComboBox, so this is our workaround
        int oldIndex = groupBySelector.currentIndex();
        groupBySelector.clear();
        groupBySelector.addItem(tr("Ungrouped"), CardList::NoSort);
        groupBySelector.addItem(tr("Group by Type"), CardList::SortByMainType);
        groupBySelector.addItem(tr("Group by Mana Value"), CardList::SortByManaValue);
        groupBySelector.addItem(tr("Group by Color"), CardList::SortByColorGrouping);
        groupBySelector.setCurrentIndex(oldIndex);
    }

    {
        int oldIndex = sortBySelector.currentIndex();
        sortBySelector.clear();
        sortBySelector.addItem(tr("Unsorted"), CardList::NoSort);
        sortBySelector.addItem(tr("Sort by Name"), CardList::SortByName);
        sortBySelector.addItem(tr("Sort by Type"), CardList::SortByType);
        sortBySelector.addItem(tr("Sort by Mana Cost"), CardList::SortByManaCost);
        sortBySelector.addItem(tr("Sort by Colors"), CardList::SortByColors);
        sortBySelector.addItem(tr("Sort by P/T"), CardList::SortByPt);
        sortBySelector.addItem(tr("Sort by Set"), CardList::SortBySet);
        sortBySelector.setCurrentIndex(oldIndex);
    }

    shuffleCheckBox.setText(tr("shuffle when closing"));
    pileViewCheckBox.setText(tr("pile view"));
}

void ZoneViewWidget::stopWindowDrag()
{
    if (!draggingWindow)
        return;

    draggingWindow = false;
    ungrabMouse();
}

QPointF ZoneViewWidget::draggedWindowPos(const QPoint &screenPos,
                                        const QPointF &scenePos,
                                        const QPointF &buttonDownScenePos) const
{
    if (dragView && dragView->viewport()) {
        const QPoint vpStart = dragView->viewport()->mapFromGlobal(dragStartScreenPos);
        const QPoint vpNow = dragView->viewport()->mapFromGlobal(screenPos);
        const QPointF sceneStart = dragView->mapToScene(vpStart);
        const QPointF sceneNow = dragView->mapToScene(vpNow);
        return dragStartItemPos + (sceneNow - sceneStart);
    }

    return dragStartItemPos + (scenePos - buttonDownScenePos);
}

bool ZoneViewWidget::windowFrameEvent(QEvent *event)
{
    // Intercept and move the widget in a stable coordinate space
    switch (event->type()) {
    case QEvent::UngrabMouse:
        stopWindowDrag();
        break;
    case QEvent::GraphicsSceneMousePress: {
        auto *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        const Qt::WindowFrameSection section = windowFrameSectionAt(me->pos());
        if (me->button() == Qt::LeftButton && section == Qt::TitleBarArea) {
            // avoid drag on close button
            const QRectF frameRectF = windowFrameRect();
            const QRectF closeRect(frameRectF.right() - kTitleBarHeight, frameRectF.top(), kTitleBarHeight,
                                   kTitleBarHeight);
            if (closeRect.contains(me->pos())) {
                me->accept();
                close();
                return true;
            }

            draggingWindow = true;
            dragStartItemPos = pos();
            dragStartScreenPos = me->screenPos();

            dragView = nullptr;
            if (me->widget()) {
                QWidget *current = me->widget();
                while (current && !dragView) {
                    dragView = qobject_cast<QGraphicsView *>(current);
                    current = current->parentWidget();
                }
            }
            if (!dragView && scene() && !scene()->views().isEmpty()) {
                dragView = scene()->views().constFirst();
            }

            // need to grab mouse to receive events and not miss initial movement
            grabMouse();

            me->accept();
            return true;
        }
        break;
    }
    case QEvent::GraphicsSceneMouseMove: {
        auto *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        if (draggingWindow) {
            if (!(me->buttons() & Qt::LeftButton)) {
                stopWindowDrag();
                me->accept();
                return true;
            }

            setPos(draggedWindowPos(me->screenPos(), me->scenePos(), me->buttonDownScenePos(Qt::LeftButton)));
            me->accept();
            return true;
        }
        break;
    }
    case QEvent::GraphicsSceneMouseRelease: {
        auto *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        if (draggingWindow && me->button() == Qt::LeftButton) {
            stopWindowDrag();
            me->accept();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return QGraphicsWidget::windowFrameEvent(event);
}

void ZoneViewWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // move if the scene routes moves while dragging
    if (draggingWindow && (event->buttons() & Qt::LeftButton)) {
        setPos(draggedWindowPos(event->screenPos(), event->scenePos(), event->buttonDownScenePos(Qt::LeftButton)));
        event->accept();
        return;
    }

    QGraphicsWidget::mouseMoveEvent(event);
}

void ZoneViewWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (draggingWindow && event->button() == Qt::LeftButton) {
        stopWindowDrag();
        event->accept();
        return;
    }

    QGraphicsWidget::mouseReleaseEvent(event);
}

QVariant ZoneViewWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange && scene()) {
        // Keep grab area in main view
        const QRectF sceneRect = scene()->sceneRect();
        const QPointF requestedPos = value.toPointF();
        QPointF desiredPos = requestedPos;

        const qreal minX = sceneRect.left();
        const qreal maxX = qMax(minX, sceneRect.right() - kMinVisibleWidth);
        const qreal minY = sceneRect.top() + kTitleBarHeight;
        const qreal maxY = qMax(minY, sceneRect.bottom() - kTitleBarHeight);

        desiredPos.setX(qBound(minX, desiredPos.x(), maxX));
        desiredPos.setY(qBound(minY, desiredPos.y(), maxY));
        return desiredPos;
    }

    return QGraphicsWidget::itemChange(change, value);
}

void ZoneViewWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    // We need to manually resize the scroll bar whenever the window gets resized
    resizeScrollbar(event->newSize().height() - extraHeight - 10);
}

void ZoneViewWidget::resizeScrollbar(const qreal newZoneHeight)
{
    qreal totalZoneHeight = zone->getOptimumRect().height();
    qreal newMax = qMax(totalZoneHeight - newZoneHeight, 0.0);
    scrollBar->setMaximum(newMax);
}

/**
 * Maps a height that is given as number of rows of cards to the actual height, given in pixels.
 *
 * @param rows Rows of cards
 * @return The height in pixels
 */
static qreal rowsToHeight(int rows)
{
    const qreal cardsHeight = (rows + 1) * (CARD_HEIGHT / 3);
    return cardsHeight + 5; // +5 padding to make the cutoff look nicer
}

/**
 * Calculates the max initial height from the settings.
 * The max initial height setting is given as number of rows, so we need to map it to a height.
 **/
static qreal calcMaxInitialHeight()
{
    return rowsToHeight(SettingsCache::instance().getCardViewInitialRowsMax());
}

/**
 * @brief Handles edge cases in determining the next default zone height. We want the height to snap when the number of
 * rows changes, but not if the player has already expanded the window.
 */
static qreal determineNewZoneHeight(qreal oldZoneHeight)
{
    // don't snap if window is taller than max initial height
    if (oldZoneHeight > calcMaxInitialHeight()) {
        return oldZoneHeight;
    }

    return calcMaxInitialHeight();
}

void ZoneViewWidget::resizeToZoneContents(bool forceInitialHeight)
{
    QRectF zoneRect = zone->getOptimumRect();
    qreal totalZoneHeight = zoneRect.height();

    qreal width = qMax(QGraphicsWidget::layout()->effectiveSizeHint(Qt::MinimumSize, QSizeF()).width(),
                       zoneRect.width() + scrollBar->width() + 10);

    QSizeF maxSize(width, zoneRect.height() + extraHeight + 10);

    qreal currentZoneHeight = rect().height() - extraHeight - 10;
    qreal newZoneHeight = forceInitialHeight ? calcMaxInitialHeight() : determineNewZoneHeight(currentZoneHeight);

    QSizeF initialSize(width, newZoneHeight + extraHeight + 10);

    setMaximumSize(maxSize);
    resize(initialSize);
    resizeScrollbar(newZoneHeight);

    zone->setGeometry(QRectF(0, -scrollBar->value(), zoneContainer->size().width(), totalZoneHeight));

    if (layout())
        layout()->invalidate();
}

void ZoneViewWidget::handleScrollBarChange(int value)
{
    zone->setY(-value);
}

void ZoneViewWidget::closeEvent(QCloseEvent *event)
{
    stopWindowDrag();
    disconnect(zone, &ZoneViewZone::closed, this, 0);
    // manually call zone->close in order to remove it from the origZones views
    zone->close();
    if (shuffleCheckBox.isChecked())
        player->getPlayerActions()->sendGameCommand(Command_Shuffle());
    zoneDeleted();
    event->accept();
}

void ZoneViewWidget::zoneDeleted()
{
    emit closePressed(this);
    deleteLater();
}

void ZoneViewWidget::initStyleOption(QStyleOption *option) const
{
    QStyleOptionTitleBar *titleBar = qstyleoption_cast<QStyleOptionTitleBar *>(option);
    if (titleBar)
        titleBar->icon = QPixmap("theme:cockatrice");
}

/**
 * Expands/shrinks the window, depending on the current height as well as the configured initial and expanded max
 * heights.
 */
void ZoneViewWidget::expandWindow()
{
    qreal maxInitialHeight = calcMaxInitialHeight();
    qreal maxExpandedHeight = rowsToHeight(SettingsCache::instance().getCardViewExpandedRowsMax());
    qreal height = rect().height() - extraHeight - 10;
    qreal maxHeight = maximumHeight() - extraHeight - 10;

    // reset window to initial max height if...
    bool doResetSize =
        // current height is less than that
        (height < maxInitialHeight) ||
        // current height is at expanded max height
        (height == maxExpandedHeight) ||
        // current height is at actual max height, and actual max height is less than expanded max height
        (height == maxHeight && height > maxInitialHeight && height < maxExpandedHeight);

    if (doResetSize) {
        resizeToZoneContents(true);
    } else {
        // expand/shrink window to expanded max height if current height is anywhere at or between initial max height
        // and actual max height
        resize(maximumSize().boundedTo({maximumWidth(), maxExpandedHeight + extraHeight + 10}));
    }
}

void ZoneViewWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->pos().y() <= 0) {
        expandWindow();
    }
}