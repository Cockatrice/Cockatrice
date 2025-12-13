#include "tutorial_overlay.h"

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>

TutorialOverlay::TutorialOverlay(QWidget *parent) : QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    // This ensures the overlay stays exactly over the parent
    if (parent) {
        QRect r = parent->rect();

        // convert the parent’s rect to screen coordinates
        QPoint globalTopLeft = parent->mapToGlobal(QPoint(0, 0));
        r.moveTopLeft(globalTopLeft);

        setGeometry(r);
    }

    bubble = new BubbleWidget(this);
    bubble->hide();

    connect(bubble->nextStepButton, &QPushButton::clicked, this, &TutorialOverlay::nextStep);
    connect(bubble->previousStepButton, &QPushButton::clicked, this, &TutorialOverlay::prevStep);
    connect(bubble->previousSequenceButton, &QPushButton::clicked, this, &TutorialOverlay::prevSequence);
    connect(bubble->nextSequenceButton, &QPushButton::clicked, this, &TutorialOverlay::nextSequence);
    connect(bubble->closeButton, &QPushButton::clicked, this, &TutorialOverlay::skipTutorial);
}

void TutorialOverlay::setTargetWidget(QWidget *w)
{
    targetWidget = w;
    update();
}

void TutorialOverlay::setText(const QString &t)
{
    tutorialText = t;
    bubble->setText(tutorialText);
    bubble->adjustSize(); // let layout recalc sizes
    QSize bsize = bubble->sizeHint();

    const QSize minSize(160, 60);
    if (bsize.width() < minSize.width()) {
        bsize.setWidth(minSize.width());
    }
    if (bsize.height() < minSize.height()) {
        bsize.setHeight(minSize.height());
    }

    // Compute the bubble rect from the current target hole
    QRect hole = targetRectOnOverlay().adjusted(-6, -6, 6, 6);
    highlightBubbleRect = computeBubbleRect(hole, bsize);

    bubble->setGeometry(highlightBubbleRect);
    bubble->raise();
    bubble->show();

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

QRect TutorialOverlay::computeBubbleRect(const QRect &hole, const QSize &bubbleSize) const
{
    const int margin = 16;
    QRect r = rect(); // overlay bounds
    QRect bubble;

    // Try right
    bubble = QRect(hole.right() + margin, hole.top(), bubbleSize.width(), bubbleSize.height());
    if (r.contains(bubble)) {
        return bubble;
    }

    // Try left
    bubble = QRect(hole.left() - margin - bubbleSize.width(), hole.top(), bubbleSize.width(), bubbleSize.height());
    if (r.contains(bubble)) {
        return bubble;
    }

    // Try above, centered
    bubble = QRect(hole.center().x() - bubbleSize.width() / 2, hole.top() - margin - bubbleSize.height(),
                   bubbleSize.width(), bubbleSize.height());
    if (r.contains(bubble)) {
        return bubble;
    }

    // Try below, centered
    bubble = QRect(hole.center().x() - bubbleSize.width() / 2, hole.bottom() + margin, bubbleSize.width(),
                   bubbleSize.height());
    if (r.contains(bubble)) {
        return bubble;
    }

    // Last-resort: clamp inside overlay
    bubble.moveLeft(std::max(r.left(), std::min(bubble.left(), r.right() - bubbleSize.width())));
    bubble.moveTop(std::max(r.top(), std::min(bubble.top(), r.bottom() - bubbleSize.height())));
    bubble.setSize(bubbleSize);
    return bubble;
}

void TutorialOverlay::parentResized()
{
    if (parentWidget()) {
        QRect r = parentWidget()->rect();
        QPoint globalTopLeft = parentWidget()->mapToGlobal(QPoint(0, 0));
        r.moveTopLeft(globalTopLeft);
        setGeometry(r);
    }
}

void TutorialOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QColor overlay(0, 0, 0, 160);
    p.fillRect(rect(), overlay);

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

    // recompute bubble size/position in case available geometry changed:
    bubble->adjustSize();
    QSize bsize = bubble->sizeHint();
    const QSize minSize(160, 60);
    if (bsize.width() < minSize.width())
        bsize.setWidth(minSize.width());
    if (bsize.height() < minSize.height())
        bsize.setHeight(minSize.height());

    highlightBubbleRect = computeBubbleRect(hole, bsize);
    bubble->setGeometry(highlightBubbleRect);
    bubble->raise();
    bubble->show();
}