#include "playmat_preview_widget.h"

#include "../cards/art_crop_attribution.h"
#include "playmat_utils.h"

#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <cmath>

namespace
{
// Mirrors the dialog/proto clamps so gestures can never produce an
// out of range parameter. The zoom FLOOR is dynamic: see
// playmatClampedZoom(), zooming out stops where the sampling window would
// exceed the card itself, so there is no dead range at the bottom end.
constexpr qreal kMaxMargin = 0.95;
constexpr qreal kMaxZoom = 4.0;
// Range of in game stack+table aspect ratios worth designing for, derived
// from PlayerGraphicsItem::paint()'s combinedArea = stack ∪ table:
//   height = 10 + 30 + 3*102 + 2*30 = 406            (TableZone rows)
//   width  = 1.5*72 + (20 + 5*72 + 15) = 503         (StackZone + TableZone
//                                                     at MIN_WIDTH)
// The area's shape depends on GAME CONTENT (played card columns widen the
// table by ~107 px each), not on the window size. Fresh board ≈ 503/406,
// a table grown to roughly double its minimum width ≈ 2.2.
constexpr qreal kMinTableAspect = 503.0 / 406.0; // fresh board: most generous framing
constexpr qreal kMaxTableAspect = 2.2;           // well developed, wide table
// Keyboard nudge steps (viewport convention: Down looks further down).
constexpr qreal kKeyPanMarginStep = 0.005;
constexpr qreal kKeyPanOffsetStep = 0.01;
constexpr qreal kKeyZoomStep = 1.05;
constexpr qreal kWheelZoomBase = 1.15; // zoom factor per wheel notch
} // namespace

PlaymatPreviewWidget::PlaymatPreviewWidget(QWidget *parent) : QWidget(parent)
{
    // The crop is square and drawn contain fit, so the height decides its
    // on screen size, keep it generous but let the dialog compress on small
    // or high DPI screens
    setMinimumSize(400, 180);
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(sp);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::OpenHandCursor);
    setAccessibleName(tr("Playmat crop"));
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
}

void PlaymatPreviewWidget::setPixmap(const QPixmap &pixmap)
{
    sourcePixmap = pixmap;
    setCursor(sourcePixmap.isNull() ? Qt::ArrowCursor : Qt::OpenHandCursor);
    update();
}

void PlaymatPreviewWidget::setParams(const PlaymatParams &p)
{
    params = p;
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
    update();
}

void PlaymatPreviewWidget::setAttribution(const QString &attribution)
{
    attributionText = attribution;
    update();
}

QRectF PlaymatPreviewWidget::activePlayArea() const
{
    // The viewport is a frame shaped like a fresh board's stack+table area
    // (kMinTableAspect): the most generous framing the game will produce.
    // Dimmed strips mark where a wider, developed table crops further.
    const QRectF cardRect = QRectF(rect()).adjusted(3, 2, -3, -2);
    return aspectFitRect(cardRect.adjusted(6, 4, -4, -4), kMinTableAspect);
}

qreal PlaymatPreviewWidget::samplingWindowSide() const
{
    if (sourcePixmap.isNull()) {
        return 0.0;
    }
    // Same clamped window the render path uses, gestures and painting must
    // never disagree about geometry.
    return playmatWindowSide(sourcePixmap.size(), params);
}

qreal PlaymatPreviewWidget::widgetToSourceScale() const
{
    const qreal cropSide = samplingWindowSide();
    const QRectF area = activePlayArea();
    if (cropSide <= 0.0 || area.isEmpty()) {
        return 0.0;
    }
    // Mirror coverFitRect(): the square crop into the (wider) viewport fills
    // its width.
    return area.width() / cropSide;
}

void PlaymatPreviewWidget::applyCropDelta(qreal dMarginL, qreal dMarginR, qreal dOffset, qreal zoomFactor)
{
    PlaymatParams next = params;
    if (dMarginL + dMarginR == 0.0) {
        // Pure horizontal pan rebalances the margins along their
        // sum constant segment. Individual bounds must not break that
        // invariant, otherwise repeated corner drags let one margin grow
        // without end, collapsing the viewing window and desynchronizing the
        // visual zoom from the readout.
        const qreal sum = params.marginPctL + params.marginPctR;
        const qreal lo = qMax(0.0, sum - kMaxMargin);
        const qreal hi = qMin(sum, kMaxMargin);
        next.marginPctL = qBound(lo, params.marginPctL + dMarginL, hi);
        next.marginPctR = sum - next.marginPctL;
    } else {
        next.marginPctL = qBound(0.0, params.marginPctL + dMarginL, kMaxMargin);
        next.marginPctR = qBound(0.0, params.marginPctR + dMarginR, kMaxMargin);
    }
    next.verticalOffset = qBound(0.0, params.verticalOffset + dOffset, 1.0);
    // Clamp through the shared helper so the floor tracks the new margins:
    // zooming out stops exactly where the window reaches the card bounds.
    next.zoom = params.zoom * zoomFactor;
    if (!sourcePixmap.isNull()) {
        next.zoom = playmatClampedZoom(sourcePixmap.size(), next);
    }

    if (sameCrop(next, params)) {
        return;
    }

    params = next;
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
    update();
    emit paramsEdited(params);
}

bool PlaymatPreviewWidget::sameCrop(const PlaymatParams &a, const PlaymatParams &b) const
{
    // Exact comparison on purpose: clamped assignments yield identical bits,
    // while qFuzzyCompare based equality misbehaves around zero
    return a.marginPctL == b.marginPctL && a.marginPctR == b.marginPctR && a.verticalOffset == b.verticalOffset &&
           a.zoom == b.zoom;
}

void PlaymatPreviewWidget::restoreSnapshot()
{
    // The snapshot only ever holds values that passed the gesture clamps,
    // so it is safe to restore verbatim
    if (sameCrop(paramsAtFocusIn, params)) {
        return;
    }
    params = paramsAtFocusIn;
    setAccessibleDescription(tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
    update();
    emit paramsEdited(params);
}

void PlaymatPreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const QRect rect = this->rect();
    const QColor accentColor(100, 116, 139);

    // Background
    const QRectF cardRect = QRectF(rect).adjusted(3, 2, -3, -2);
    QLinearGradient bg(cardRect.topLeft(), cardRect.topRight());
    bg.setColorAt(0, accentColor.darker(320));
    bg.setColorAt(1, QColor(18, 22, 30));
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);
    painter.drawRoundedRect(cardRect, 6, 6);
    painter.setBrush(accentColor);
    painter.drawRoundedRect(QRectF(cardRect.left(), cardRect.top(), 3, cardRect.height()), 2, 2);

    if (sourcePixmap.isNull()) {
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(rect, Qt::AlignCenter, tr("No card selected"));
        return;
    }

    const QRectF playArea = activePlayArea();

    // Exactly the game's pipeline (player_graphics_item): cover fit the crop
    // into the table shaped viewport, centered, so the frame shows precisely
    // what a minimum aspect window shows, and dragging moves the art behind
    // the fixed frame.
    const QRectF srcRect = PlaymatUtils::computeArtSourceRect(sourcePixmap.size(), params);
    const QRectF dstRect = PlaymatUtils::coverFitRect(playArea, srcRect.size());

    painter.setClipRect(playArea.toRect());
    painter.drawPixmap(dstRect, sourcePixmap, srcRect);

    // Wider (developed) tables crop further: mark where a kMaxTableAspect
    // board stops. Palette driven so theme authors can recolor the markers.
    const qreal wideBandHeight = playArea.height() * (kMinTableAspect / kMaxTableAspect);
    const qreal stripHeight = (playArea.height() - wideBandHeight) / 2.0;
    QColor stripColor = palette().color(QPalette::Window);
    stripColor.setAlpha(150);
    painter.fillRect(QRectF(playArea.left(), playArea.top(), playArea.width(), stripHeight), stripColor);
    painter.fillRect(QRectF(playArea.left(), playArea.bottom() - stripHeight, playArea.width(), stripHeight),
                     stripColor);
    QColor hairlineColor = palette().color(QPalette::Highlight);
    hairlineColor.setAlpha(110);
    painter.setPen(QPen(hairlineColor, 1));
    painter.drawLine(QPointF(playArea.left(), playArea.top() + stripHeight),
                     QPointF(playArea.right(), playArea.top() + stripHeight));
    painter.drawLine(QPointF(playArea.left(), playArea.bottom() - stripHeight),
                     QPointF(playArea.right(), playArea.bottom() - stripHeight));

    // Draw zone divider: stack is roughly the left portion
    const double stackWidthRatio = 0.18; // Stack is about 18% of total play area
    const double stackDividerX = playArea.left() + playArea.width() * stackWidthRatio;

    // Subtle semi transparent overlays to distinguish zones
    // Stack zone overlay (slightly darker)
    QRectF stackOverlay(playArea.left(), playArea.top(), playArea.width() * stackWidthRatio, playArea.height());
    painter.fillRect(stackOverlay, QColor(0, 0, 0, 40));

    // Table zone overlay (very subtle)
    QRectF tableOverlay(stackDividerX, playArea.top(), playArea.width() * (1.0 - stackWidthRatio), playArea.height());
    painter.fillRect(tableOverlay, QColor(0, 0, 0, 20));

    // Zone divider line
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
    painter.drawLine(QPointF(stackDividerX, playArea.top()), QPointF(stackDividerX, playArea.bottom()));

    // Land divider line (about 60% down the table area)
    const double landDividerY = playArea.top() + playArea.height() * 0.65;
    painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
    painter.drawLine(QPointF(stackDividerX, landDividerY), QPointF(playArea.right(), landDividerY));
    painter.setClipping(false);

    // Border around the viewport = boundary of every plausible framing.
    painter.setPen(QPen(QColor(70, 80, 95, 120), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(playArea.adjusted(0, 0, -1, -1), 3, 3);

    // Visible keyboard focus per the focus cursor contract, Tab must show
    // where the keys land
    if (hasFocus()) {
        QPen focusPen(palette().color(QPalette::Highlight), 2);
        painter.setPen(focusPen);
        painter.drawRoundedRect(playArea.adjusted(-1, -1, 1, 1), 3, 3);
    }

    paintArtAttribution(painter, playArea, attributionText, Qt::AlignRight | Qt::AlignBottom, 0.8);

    // Zoom readout so the gesture has a visible, stable counterpart.
    QColor ink = palette().color(QPalette::WindowText);
    ink.setAlpha(160);
    painter.setPen(ink);
    painter.drawText(QPointF(playArea.left() + 8, playArea.bottom() - 8),
                     tr("Zoom %1×").arg(QString::number(params.zoom, 'f', 2)));
}

void PlaymatPreviewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || sourcePixmap.isNull() || samplingWindowSide() <= 0.0) {
        QWidget::mousePressEvent(event);
        return;
    }
    lastDragPos = event->pos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void PlaymatPreviewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) || sourcePixmap.isNull()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPointF delta = QPointF(event->pos() - lastDragPos);
    lastDragPos = event->pos();

    const qreal scale = widgetToSourceScale();
    // Vertical travel of the SAMPLING window: verticalOffset moves its top
    // edge by exactly this much per unit, identical to the render path.
    // Windows taller than the art (square/landscape sources zoomed out)
    // leave no travel, vertical drags are then boundary no ops.
    const qreal travel = static_cast<qreal>(sourcePixmap.height()) - samplingWindowSide();
    if (scale <= 0.0) {
        event->accept();
        return;
    }

    // Dragging moves the ART with the cursor, so the viewing window slides the
    // other way. Horizontal panning rebalances the margins (their sum, hence
    // the window width, stays constant), vertical panning moves the window's
    // top edge within its available travel.
    const qreal sourceW = sourcePixmap.width();
    const qreal dMargin = -(delta.x() / scale) / sourceW;
    const qreal dOffset = travel > 0.5 ? -(delta.y() / scale) / travel : 0.0;

    applyCropDelta(dMargin, -dMargin, dOffset, 1.0);
    event->accept();
}

void PlaymatPreviewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(sourcePixmap.isNull() ? Qt::ArrowCursor : Qt::OpenHandCursor);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void PlaymatPreviewWidget::wheelEvent(QWheelEvent *event)
{
    if (sourcePixmap.isNull() || samplingWindowSide() <= 0.0) {
        QWidget::wheelEvent(event);
        return;
    }
    const qreal notches = static_cast<qreal>(event->angleDelta().y()) / 120.0;
    if (notches == 0.0) {
        event->accept();
        return;
    }
    applyCropDelta(0.0, 0.0, 0.0, std::pow(kWheelZoomBase, notches));
    event->accept();
}

void PlaymatPreviewWidget::keyPressEvent(QKeyEvent *event)
{
    if (sourcePixmap.isNull()) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
        case Qt::Key_Escape:
            if (sameCrop(params, paramsAtFocusIn)) {
                // Nothing to undo on this surface, let the event reach the
                // dialog so Esc keeps its close meaning there
                QWidget::keyPressEvent(event);
                return;
            }
            restoreSnapshot();
            break;
        case Qt::Key_Backspace:
            restoreSnapshot();
            break;
        case Qt::Key_Left:
            applyCropDelta(-kKeyPanMarginStep, kKeyPanMarginStep, 0.0, 1.0);
            break;
        case Qt::Key_Right:
            applyCropDelta(kKeyPanMarginStep, -kKeyPanMarginStep, 0.0, 1.0);
            break;
        case Qt::Key_Up:
            applyCropDelta(0.0, 0.0, -kKeyPanOffsetStep, 1.0);
            break;
        case Qt::Key_Down:
            applyCropDelta(0.0, 0.0, kKeyPanOffsetStep, 1.0);
            break;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            applyCropDelta(0.0, 0.0, 0.0, kKeyZoomStep);
            break;
        case Qt::Key_Minus:
            applyCropDelta(0.0, 0.0, 0.0, 1.0 / kKeyZoomStep);
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
    }
    event->accept();
}

void PlaymatPreviewWidget::focusInEvent(QFocusEvent *event)
{
    // Snapshot for the Esc or Backspace reset, restoring whatever the user
    // had when the surface took focus
    paramsAtFocusIn = params;
    QWidget::focusInEvent(event);
}
