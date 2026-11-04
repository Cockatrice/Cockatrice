#include "card_database_visual_view.h"

#include "../../card_picture_loader/card_picture_loader.h"
#include "../cards/card_info_picture_widget.h"
#include "../cards/card_picture_delegate.h"
#include "../cards/card_size_widget.h"
#include "card_database_visual_display_model.h"
#include "libcockatrice/card/printing/exact_card.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QModelIndex>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSize>
#include <QWheelEvent>
#include <QWidget>
#include <QtGlobal>
#include <qnamespace.h>

CardDatabaseVisualView::CardDatabaseVisualView(CardDatabaseVisualDisplayModel *_model, QWidget *parent)
    : QListView(parent), model(_model)
{
    auto *cardDelegate = new CardPictureDelegate(model, this);
    setItemDelegate(cardDelegate);
    setModel(model);

    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setUniformItemSizes(true);
    setLayoutMode(QListView::SinglePass);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setWordWrap(false);

    // QListView hands the delegate's item height to the scroll bar as its single
    // step under ScrollPerPixel, so one wheel notch would jump wheelScrollLines()
    // whole card rows. Give the bar a text line instead; the wheel handler below
    // scales that line to the grid. A positive step also stops Qt from taking the
    // item height back on the next layout.
    verticalScrollBar()->setSingleStep(qMax(1, fontMetrics().height()));

    // The grid is a read-only surface: clicks are actions on a card, not a
    // selection, and they must not steal focus from the search field.
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);

    // Hover drives the card preview.
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    connect(&CardPictureLoader::getInstance(), &CardPictureLoader::pixmapLoaded, this,
            [this] { viewport()->update(); });

    // A new search result starts at the top, like the widget-per-card grid did.
    connect(model, &QAbstractItemModel::modelReset, this, [this] {
        verticalScrollBar()->setValue(0);
        // The rows under a stationary cursor changed with the reset: re-resolve the
        // hover so the preview follows the grid, as enterEvent did per card widget.
        hoveredRow = -1;
        updateHoveredRow(viewport()->mapFromGlobal(QCursor::pos()));
    });

    setCardSizeFactor(cardSizeFactor);
}

void CardDatabaseVisualView::setCardSizeFactor(int factor)
{
    cardSizeFactor = qBound(CardSizeWidget::MIN_CARD_SIZE, factor, CardSizeWidget::MAX_CARD_SIZE);

    const QSize cardSize = CardInfoPictureWidget::sizeForScaleFactor(cardSizeFactor);
    if (auto *cardDelegate = qobject_cast<CardPictureDelegate *>(itemDelegate())) {
        cardDelegate->setCardSize(cardSize);
    }
    // The delegate's sizeHint does not emit sizeHintChanged, so the grid size is what
    // makes a resize take effect. Keep the two in step.
    setGridSize(cardSize + QSize(CardPictureDelegate::SPACING, CardPictureDelegate::SPACING));
}

void CardDatabaseVisualView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid() && model != nullptr) {
        const ExactCard card = model->cardAt(index.row());
        if (card) {
            emit cardClicked(event, card);
            event->accept();
            return;
        }
    }

    QListView::mousePressEvent(event);
}

void CardDatabaseVisualView::mouseMoveEvent(QMouseEvent *event)
{
    QListView::mouseMoveEvent(event);
    updateHoveredRow(event->pos());
}

bool CardDatabaseVisualView::viewportEvent(QEvent *event)
{
    // A per-card widget reported its hover from enterEvent, which also fires when the
    // grid changes under a stationary cursor. A QListView only reports moves, so
    // re-resolve the hover on enter and clear it on leave.
    if (event->type() == QEvent::Enter) {
        updateHoveredRow(viewport()->mapFromGlobal(QCursor::pos()));
    } else if (event->type() == QEvent::Leave) {
        hoveredRow = -1;
    }

    return QListView::viewportEvent(event);
}

void CardDatabaseVisualView::wheelEvent(QWheelEvent *event)
{
    // Ctrl+wheel is the card-size gesture and is filtered out before it reaches
    // the view; any other modifier keeps QAbstractSlider's own page scrolling.
    if (event->modifiers() != Qt::NoModifier) {
        QListView::wheelEvent(event);
        return;
    }

    // macOS uses transient (overlay) scroll bars that hide themselves
    // once they fade out, so we need to use the range to determine if we ignore the wheel when there is nothing to
    // scroll. This lets an enclosing scroll area take over.
    QScrollBar *bar = verticalScrollBar();
    if (bar->maximum() <= bar->minimum()) {
        event->ignore();
        return;
    }

    // Touchpads and high-resolution wheels already report pixels: follow them.
    if (!event->pixelDelta().isNull()) {
        bar->setValue(bar->value() - event->pixelDelta().y());
        event->accept();
        return;
    }

    if (event->angleDelta().y() == 0) {
        QListView::wheelEvent(event);
        return;
    }

    // Accumulate wheel deltas smaller than a notch, like QAbstractSlider does, so
    // a fine-grained wheel still moves.
    wheelRemainder += static_cast<qreal>(event->angleDelta().y()) / 120.0;
    const int notches = static_cast<int>(wheelRemainder);
    if (notches == 0) {
        event->accept();
        return;
    }
    wheelRemainder -= notches;

    const int pixels = -notches * wheelStep();
    bar->setValue(qBound(bar->minimum(), bar->value() + pixels, bar->maximum()));
    event->accept();
}

int CardDatabaseVisualView::wheelStep() const
{
    // A card row is far too coarse for one wheel line, and a text line is too
    // fine, so a line is an eighth of the grid cell. A notch then moves about a
    // third of a card row at any card size, and the OS line count still applies.
    const int lineHeight = qMax(1, gridSize().height() / 8);
    return qMax(1, QApplication::wheelScrollLines()) * lineHeight;
}

void CardDatabaseVisualView::updateHoveredRow(const QPoint &position)
{
    // indexAt() also answers for points outside the viewport, so a cursor parked
    // below the grid (on the size slider, say) would resolve to the row just under
    // the visible area. Only a point inside the viewport can hover a card.
    if (model == nullptr || !viewport()->rect().contains(position)) {
        return;
    }

    const QModelIndex index = indexAt(position);
    const int row = index.isValid() ? index.row() : -1;
    if (row == hoveredRow) {
        return;
    }

    hoveredRow = row;
    if (row >= 0) {
        const ExactCard card = model->cardAt(row);
        if (card) {
            emit cardHovered(card);
        }
    }
}

void CardDatabaseVisualView::leaveEvent(QEvent *event)
{
    hoveredRow = -1;
    QListView::leaveEvent(event);
}
