#include "tutorial_overlay.h"

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>

TutorialOverlay::TutorialOverlay(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    // This ensures the overlay stays exactly over the parent
    if (parent) {
        setGeometry(parent->rect());
    }
}

void TutorialOverlay::setTargetWidget(QWidget *w)
{
    targetWidget = w;
    update();
}

void TutorialOverlay::setText(const QString &t)
{
    tutorialText = t;
    update();
}

void TutorialOverlay::showEvent(QShowEvent *)
{
    raise();
}

void TutorialOverlay::resizeEvent(QResizeEvent *)
{
    update();
}

QRect TutorialOverlay::targetRectOnOverlay() const
{
    if (!targetWidget) {
        return QRect();
    }

    // Widget -> global screen coordinates
    QPoint globalTopLeft = targetWidget->mapToGlobal(QPoint(0, 0));

    // Global -> overlay-local coordinates
    QPoint localTopLeft = mapFromGlobal(globalTopLeft);

    return QRect(localTopLeft, targetWidget->size());
}

QRect TutorialOverlay::computeBubbleRect(const QRect &hole) const
{
    const int bubbleW = 250;
    const int bubbleH = 120;
    const int margin = 16;

    QRect r = rect(); // overlay bounds
    QRect bubble;

    // Try right
    bubble = QRect(hole.right() + margin, hole.top(), bubbleW, bubbleH);
    if (r.contains(bubble)) {
        return bubble;
    }

    // Try left
    bubble = QRect(hole.left() - margin - bubbleW, hole.top(), bubbleW, bubbleH);
    if (r.contains(bubble)) {
        return bubble;
    }

    // Try above, centered
    bubble = QRect(hole.center().x() - bubbleW / 2, hole.top() - margin - bubbleH, bubbleW, bubbleH);
    if (r.contains(bubble)) {
        return bubble;
    }

    // Try below, centered
    bubble = QRect(hole.center().x() - bubbleW / 2, hole.bottom() + margin, bubbleW, bubbleH);
    if (r.contains(bubble)) {
        return bubble;
    }

    // Last-resort: clamp inside overlay
    bubble.moveLeft(std::max(r.left(), std::min(bubble.left(), r.right() - bubbleW)));
    bubble.moveTop(std::max(r.top(), std::min(bubble.top(), r.bottom() - bubbleH)));
    return bubble;
}

void TutorialOverlay::parentResized()
{
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
    }
}

void TutorialOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Darken the screen
    QColor overlay(0, 0, 0, 160);
    p.fillRect(rect(), overlay);

    // Highlight hole
    QRect hole = targetRectOnOverlay().adjusted(-6, -6, 6, 6);
    if (!hole.isEmpty()) {
        QPainterPath path;
        path.addRect(rect());
        QPainterPath holePath;
        holePath.addRoundedRect(hole, 8, 8);
        path = path.subtracted(holePath);

        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillPath(holePath, Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // Draw bubble
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255));
    highlightBubbleRect = computeBubbleRect(hole);
    p.drawRoundedRect(highlightBubbleRect, 8, 8);

    // Text
    p.setPen(Qt::black);
    p.drawText(highlightBubbleRect.adjusted(10, 10, -10, -10), Qt::TextWordWrap, tutorialText);
}

void TutorialOverlay::mousePressEvent(QMouseEvent *ev)
{
    // Clicks inside bubble → next
    if (highlightBubbleRect.contains(ev->pos())) {
        emit nextStep();
        return;
    }

    // Click anywhere else means skip
    emit skipTutorial();
}